// Recursively pack directory contents into a BDP file
// Compile cmdlet:
// clang -std=c11 -Wall -Wextra -O2 tools/asset-packer.c -I include -o tools/asset-packer (subject to change due to linking)
//

#include <lz4/lz4.h>
#include <xxhash/xxhash.h>
#include <bdp_format.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// I refuse to work with windows, when compiling use mingw.
#include <dirent.h>
#include <sys/stat.h>

typedef struct {
	char 		*path;
	char 		*normalized_path;
	XXH64_hash_t	cluster_hash;
} FileInfo;

FILE 		*packed_output;
FILE 		*cluster_output;

BchEntry 	*cluster_register;
size_t		cluster_count = 0;
uint32_t	cluster_cursor = 0;

FileInfo 	*file_register;
size_t		file_count = 0;

BdpChunk 	*chunk_register = NULL;
uint32_t 	chunk_count = 0;

BdpEntry 	*entry_register = NULL;
uint32_t	chunks_offset = 0;

static AssetType retrieve_asset_type(char *normalized_path)
{
	char *dot = strrchr(normalized_path, '.');

	if (!dot || dot==normalized_path) return ASSET_TYPE_INVALID;
	dot++;

	if (!strcmp(dot, "png")) return ASSET_TYPE_TEXTURE;
	if (!strcmp(dot, "ttf")) return ASSET_TYPE_FONT;
	if (!strcmp(dot, "wav")) return ASSET_TYPE_SOUND;
	if (!strcmp(dot, "ogg")) return ASSET_TYPE_SOUND;
	if (!strcmp(dot, "mp3")) return ASSET_TYPE_SOUND;
	return ASSET_TYPE_INVALID;
}

static void normalize_path(char *path)
{
	char *p = path;
	while (*p){
		if (*p == '\\') *p = '/';
		else if (*p >= 'A' && *p <= 'Z') *p += 'a' - 'A';
		p++;
	}
}

static XXH64_hash_t get_cluster_hash(const char *normalized_path)
{
	char _cluster_name[BDP_CLUSTER_NAME_LENGTH];
	const char *key = strchr(normalized_path, '/');
	if (!key){
		strcpy(_cluster_name, "root");
	}
	size_t length = key - normalized_path;
	memcpy(_cluster_name, normalized_path, length);
	_cluster_name[length] = '\0';

	return XXH64(_cluster_name, length, BINARY_HASH_SEED);
}

static void push_cluster(const char *path)
{
	XXH64_hash_t hash = get_cluster_hash(path);
	cluster_register = realloc(cluster_register, sizeof(BchEntry) * (cluster_count + 1));
	cluster_register[cluster_count].hash = hash;
	cluster_register[cluster_count].initial_chunk = -1;
	++cluster_count;
}

static int compare_clusters(const void *a, const void *b)
{
	const BchEntry *entry_a = (const BchEntry *)a;
	const BchEntry *entry_b = (const BchEntry *)b;

	if (entry_a->hash < entry_b->hash) return -1;
	else if (entry_a->hash > entry_b->hash) return 1;
	else return 0;
}

static int compare_entries(const void *a, const void *b)
{
	const BdpEntry *entry_a = (const BdpEntry *)a;
	const BdpEntry *entry_b = (const BdpEntry *)b;

	if (entry_a->hash < entry_b->hash) return -1;
	else if (entry_a->hash > entry_b->hash) return 1;
	else return 0;
}

static void sort_clusters(void)
{
	qsort(cluster_register, cluster_count, sizeof(BchEntry), compare_clusters);
}

static void sort_entries(void)
{
	qsort(entry_register, file_count, sizeof(BdpEntry), compare_entries);
}

static void push_file(const char *path, const char *normalized_path)
{
	file_register = realloc(file_register, sizeof(FileInfo) * (file_count + 1));

	file_register[file_count].path = strdup(path);
	file_register[file_count].normalized_path = strdup(normalized_path);
	file_register[file_count].cluster_hash = get_cluster_hash(normalized_path);

	fprintf(stderr, "Pushed file %s to cluster %I64u\n", normalized_path, file_register[file_count].cluster_hash);
	++file_count;
}

static void walk_directory(const char *base, const char *rel)
{
	char full[1024];
	snprintf(full, sizeof(full), "%s/%s", base, rel);

	DIR* directory = opendir(full);
	if (!directory) return;

	struct dirent *ent;
	while ((ent = readdir(directory))){
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

		char next_rel[1024];
		if (*rel) snprintf(next_rel, sizeof(next_rel), "%s/%s", rel, ent->d_name);
		else snprintf(next_rel, sizeof(next_rel), "%s", ent->d_name);

		char next_full[1024];
		snprintf(next_full, sizeof(next_full), "%s/%s", base, next_rel);

		struct stat st;
		stat(next_full, &st);

		if (S_ISDIR(st.st_mode)) walk_directory(base, next_rel);
		else if (S_ISREG(st.st_mode)) {
			normalize_path(next_rel);

			char *ext = strchr(next_rel, '.');
			const char *ext_str = ext ? ext + 1 : "";
			if (!strcmp(ext_str, BCH_TRIGGER)){
				push_cluster(next_rel);
				continue;
			}
			push_file(next_full, next_rel);
		}

	}
	closedir(directory);
}

static void pack_file(size_t index)
{
	FileInfo *file = &file_register[index];

	XXH64_hash_t file_hash = XXH64(file->normalized_path, strlen(file->normalized_path), BINARY_HASH_SEED);

	FILE *input = fopen(file->path, "rb");
	if (!input) return;

	uint32_t first_chunk = chunk_count;
	uint32_t raw_bytes = 0;

	uint8_t raw[BDP_CHUNK_RAW_SIZE];
	uint8_t compressed[LZ4_compressBound(BDP_CHUNK_RAW_SIZE)];

	while (cluster_register[cluster_cursor].hash != file->cluster_hash) cluster_cursor = (cluster_cursor + 1) % cluster_count;
	if (cluster_register[cluster_cursor].initial_chunk == UINT32_MAX) cluster_register[cluster_cursor].initial_chunk = chunk_count;

	while(1){
		size_t read = fread(raw, 1, sizeof(raw), input);
		if (!read) break;

		int comp_size = LZ4_compress_default((char *)raw, (char *)compressed, read, sizeof(compressed));

		BdpChunk chunk = {
			.data_offset = ftell(packed_output),
			.size = comp_size,
			.raw_size = read
		};

		chunks_offset += fwrite(compressed, 1, comp_size, packed_output);
		chunk_register = realloc(chunk_register, sizeof(BdpChunk) * (chunk_count + 1));
		chunk_register[chunk_count++] = chunk;

		raw_bytes += read;
	}
	fclose(input);

	BdpEntry file_entry = {
		.hash = file_hash,
		.type = retrieve_asset_type(file->normalized_path),
		.first_chunk = first_chunk,
		.chunk_count = chunk_count - first_chunk,
		.cluster_hash = file->cluster_hash,
		.raw_size = raw_bytes
	};
	entry_register[index] = file_entry;

	fprintf(stderr, "Packed file at normalized path \"%s\" with hash %I64u\n", file->normalized_path, file_entry.hash);
}

static inline void init_packing(void)
{
	BdpHeader _dummy_header = {0};
	BdpEntry _dummy_entry = {0};
	fseek(packed_output, 0, SEEK_SET);
	fwrite(&_dummy_header, sizeof(BdpHeader), 1, packed_output);
	fwrite(&_dummy_entry, sizeof(BdpEntry), file_count, packed_output);
}

static inline void write_files(void)
{
	entry_register = realloc(entry_register, sizeof(BdpEntry) * file_count);
	chunks_offset = sizeof(BdpHeader) + sizeof(BdpEntry) * file_count;
	for (size_t file_index = 0; file_index < file_count; ++file_index){
		pack_file(file_index);
	}
	for (size_t cluster = 0; cluster < cluster_count; ++cluster){
		uint32_t next =
		(cluster+1 < cluster_count)
			? cluster_register[cluster+1].initial_chunk
			: chunk_count;
		cluster_register[cluster].chunk_count = next - cluster_register[cluster].initial_chunk;
	}
}

static inline void conclude_packing(void)
{
	fwrite(chunk_register, sizeof(BdpChunk)*chunk_count, 1, packed_output);
	BdpHeader header = {
		.version = PACKER_VERSION,
		.asset_count = file_count,
		.toc_offset = sizeof(BdpHeader),
		.data_offset = sizeof(BdpHeader) + sizeof(BdpEntry) * file_count,
		.chunk_register_offset = chunks_offset
	};
	memcpy(&header.magic, BDP_MAGIC_BYTES, 4);
	fseek(packed_output, 0, SEEK_SET);
	fwrite(&header, sizeof(BdpHeader), 1, packed_output);

	sort_entries();
	fwrite(entry_register, sizeof(BdpEntry) * file_count, 1, packed_output);

	BchHeader cluster_header = {
		.version = PACKER_VERSION,
		.cluster_count = cluster_count,
	};
	memcpy(&cluster_header.magic, BCH_MAGIC_BYTES, 4);
	fwrite(&cluster_header, sizeof(BchHeader), 1, cluster_output);
	fwrite(cluster_register, sizeof(BchEntry) * cluster_count, 1, cluster_output);
}

int main(int argc, char *argv[])
{
	if (argc!=3){
		fprintf(stderr, "Usage: %s <directory_to_pack> <output_file_name>\n", argv[0]);
		return 1;
	}

	const char *asset_directory = argv[1];
	walk_directory(asset_directory, "");

	sort_clusters();

	char packed_output_file_name[strlen(argv[2]) + strlen(BDP_FILE_EXT)];
	strcpy(packed_output_file_name, argv[2]);
	strcat(packed_output_file_name, BDP_FILE_EXT);

	char cluster_output_file_name[strlen(argv[2]) + strlen(BCH_FILE_EXT)];
	strcpy(cluster_output_file_name, argv[2]);
	strcat(cluster_output_file_name, BCH_FILE_EXT);

	packed_output = fopen(packed_output_file_name, "wb+");
	cluster_output = fopen(cluster_output_file_name, "wb+");

	init_packing();
	write_files();
	conclude_packing();

	fprintf(stderr, "Successfully packed %zu files into the binary [%s]\n", file_count, packed_output_file_name);
	fprintf(stderr, "Sucessfully packed %zu clusters into the binary table [%s]\n", cluster_count, cluster_output_file_name);

	fclose(packed_output);
	fclose(cluster_output);
	free(cluster_register);
	free(file_register);
	free(chunk_register);
	free(entry_register);

	return 0;
}
