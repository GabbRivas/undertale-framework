#include <raylib.h>
#include <asset_loader/asset_loader.h>
#include <bdp_format.h>
#include <xxhash/xxhash.h>
#include <lz4/lz4.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <debug.h>
#include <pthread.h>
#include <unistd.h> //Thread-safe

static FILE *bdp_file;
static FILE *bch_file;
static BdpHeader *bdp_header;
static BchHeader *bch_header;
static BdpEntry *asset_register;
static BchEntry *cluster_register;
static ClusterState *cluster_state_register;
static CompressedChunk *chunk_register = NULL;
static uint32_t loaded_chunks = 0;

#define ARENA_SIZE (64 * 1024 * 1024)  // Tune as needed
static uint8_t *arena_base = NULL;
static size_t arena_pos = 0;
static pthread_mutex_t arena_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;
// I envy POSIX os's
// TODO: Abstract fread and fseek into a function which automatically locks the read operations readat(dst, size, cursor) [will be fixed to seek_set but its not like we use any other]

static inline void _asset_fatal(const char *message, ...)
{
	char buffer[1024];

	va_list args;
	va_start(args, message);
	vsnprintf(buffer, sizeof(buffer), message, args);
	va_end(args);

	fprintf(stderr, "%s.\n", buffer);
	debug_print("[%s] %s\n", ASSET_LOADER_SIGN, buffer);

	if (bdp_file) fclose(bdp_file);
	if (bch_file) fclose(bch_file);
	exit(EXIT_FAILURE);
}

static void *arena_alloc(size_t size)
{
	pthread_mutex_lock(&arena_lock);
	if (arena_pos + size > ARENA_SIZE) {
		pthread_mutex_unlock(&arena_lock);
		_asset_fatal("Arena exhausted");
		return NULL;
	}
	uint8_t *ptr = arena_base + arena_pos;
	arena_pos += size;
	pthread_mutex_unlock(&arena_lock);
	return ptr;
}

static char *retrieve_file_ext(const char *normalized_path)
{
	char *extension = strrchr(normalized_path, '.');
	if (!extension || extension == NULL) return "";
	return extension;
}

static char *normalize_path(const char *path)
{
	char *normalized_path = strdup(path);
	if (!normalized_path) _asset_fatal("Failed to allocate memory for path normalization");
	char *p = normalized_path;
	while (*p) {
		if (*p == '\\') *p = '/';
		else if (*p >= 'A' && *p <= 'Z') *p += 'a' - 'A';
		p++;
	}
	return normalized_path;
}

int compare_chunks(const void *a, const void *b)
{
	const CompressedChunk *chunk_a = (const CompressedChunk *)a;
	const CompressedChunk *chunk_b = (const CompressedChunk *)b;
	// int _delta = chunk_a->idx - chunk_b->idx;
	// return (_delta > 0) - (_delta < 0);

	if (chunk_a->idx < chunk_b->idx) return -1;
	else if (chunk_a->idx > chunk_b->idx) return 1;
	else return 0;
}

void sort_chunks() {
	qsort(chunk_register, loaded_chunks, sizeof(CompressedChunk), compare_chunks);
}

static int32_t cluster_find(XXH64_hash_t hash)
{
	int low = 0, high = bch_header->cluster_count - 1;
	while (low <= high) {
		int mid = low + (high - low) / 2;
		if (cluster_register[mid].hash == hash) return mid;
		else if (cluster_register[mid].hash < hash) low = mid + 1;
		else high = mid - 1;
	}
	return -1;
}

static int32_t find_chunk(uint32_t target_idx)
{
	int low = 0, high = loaded_chunks - 1;
	while (low <= high) {
		int mid = low + (high - low) / 2;
		if (chunk_register[mid].idx == target_idx) return mid;
		else if (chunk_register[mid].idx < target_idx) low = mid + 1;
		else high = mid - 1;
	}
	return -1;
}

static void *load_cluster_thread(void *arg)
{
	int32_t cluster_idx = (int32_t)(intptr_t)arg;
	BchEntry *cluster = &cluster_register[cluster_idx];

	pthread_mutex_lock(&arena_lock);
	chunk_register = realloc(chunk_register, sizeof(CompressedChunk) * (loaded_chunks + cluster->chunk_count));
	pthread_mutex_unlock(&arena_lock);
	if (!chunk_register) {
		cluster_state_register[cluster_idx].loading = 0;
		return NULL;
	}

	CompressedChunk *temp_entry = calloc(cluster->chunk_count, sizeof(CompressedChunk));
	if (!temp_entry) {
		cluster_state_register[cluster_idx].loading = 0;
		return NULL;
	}

	BdpChunk scanned_chunk = {0};
	for (uint32_t chunk = 0; chunk < cluster->chunk_count; ++chunk) {
		pthread_mutex_lock(&file_lock);
		fseek(bdp_file, bdp_header->chunk_register_offset + sizeof(BdpChunk)*(cluster->initial_chunk + chunk), SEEK_SET);
		fread(&scanned_chunk, sizeof(BdpChunk), 1, bdp_file);

		uint8_t *mem = arena_alloc(scanned_chunk.size);
		if (!mem) {
			free(temp_entry);
			cluster_state_register[cluster_idx].loading = 0;
			return NULL;
		}

		fseek(bdp_file, scanned_chunk.data_offset, SEEK_SET);
		fread(mem, scanned_chunk.size, 1, bdp_file);
		pthread_mutex_unlock(&file_lock);

		CompressedChunk new_chunk = {
			.idx = cluster->initial_chunk + chunk,
			.compressed_data = mem,
			.compressed_size = scanned_chunk.size,
			.raw_size = scanned_chunk.raw_size
		};
		temp_entry[chunk] = new_chunk;
		debug_print("[%s] Read and stored Chunk %i\n", ASSET_LOADER_SIGN, cluster->initial_chunk + chunk);
	}

	pthread_mutex_lock(&arena_lock);
	memcpy(chunk_register + loaded_chunks, temp_entry, sizeof(CompressedChunk) * cluster->chunk_count);
	loaded_chunks += cluster->chunk_count;
	pthread_mutex_unlock(&arena_lock);

	free(temp_entry);
	sort_chunks();

	cluster_state_register[cluster_idx].loading = 0;
	cluster_state_register[cluster_idx].loaded = 1;
	cluster_state_register[cluster_idx].load_done = 1;
	return NULL;
}

bool asset_system_init(const char *pack_path)
{
	char *data_binary_pack_path = malloc(strlen(pack_path) + strlen(BDP_FILE_EXT) + 1);
	char *data_cluster_header_path = malloc(strlen(pack_path) + strlen(BCH_FILE_EXT) + 1);
	strcpy(data_binary_pack_path, pack_path);
	strcpy(data_cluster_header_path, pack_path);
	strcat(data_binary_pack_path, BDP_FILE_EXT);
	strcat(data_cluster_header_path, BCH_FILE_EXT);

	bdp_file = fopen(data_binary_pack_path, "rb");
	bch_file = fopen(data_cluster_header_path, "rb");
	free(data_binary_pack_path);
	free(data_cluster_header_path);

	if (!bdp_file || !bch_file) {
		_asset_fatal("Failed to open asset pack files %s with extensions %s and %s", pack_path, BDP_FILE_EXT, BCH_FILE_EXT);
		return false;
	};
	debug_print("[%s] Pack files opened successfully.\n", ASSET_LOADER_SIGN);

	bdp_header = malloc(sizeof(BdpHeader));
	bch_header = malloc(sizeof(BchHeader));
	size_t read_bdp = fread(bdp_header, sizeof(BdpHeader), 1, bdp_file);
	size_t read_bch = fread(bch_header, sizeof(BchHeader), 1, bch_file);

	if (!read_bdp || !read_bch) {
		_asset_fatal("Could not retrieve asset binaries headers");
		fclose(bdp_file);
		fclose(bch_file);
		return false;
	}
	debug_print("[%s] Binaries headers retrieved successfully.\n", ASSET_LOADER_SIGN);

	asset_register = malloc(bdp_header->asset_count * sizeof(BdpEntry));
	cluster_register = malloc(bch_header->cluster_count * sizeof(BchEntry));
	if (!asset_register || !cluster_register) {
		_asset_fatal("Failed to allocate memory for asset registers");
		fclose(bdp_file);
		fclose(bch_file);
		return false;
	}

	fseek(bdp_file, bdp_header->toc_offset, SEEK_SET);
	fseek(bch_file, sizeof(BchHeader), SEEK_SET);
	read_bdp = fread(asset_register, sizeof(BdpEntry), bdp_header->asset_count, bdp_file);
	read_bch = fread(cluster_register, sizeof(BchEntry), bch_header->cluster_count, bch_file);

	if (!read_bdp || !read_bch) {
		_asset_fatal("Could not retrieve asset data entries");
		fclose(bdp_file);
		fclose(bch_file);
		return false;
	}
	debug_print("[%s] Data entries retrieved successfully.\n", ASSET_LOADER_SIGN);

	cluster_state_register = calloc(bch_header->cluster_count, sizeof(ClusterState));
	if (!cluster_state_register) {
		_asset_fatal("Failed to allocate memory for cluster states");
		fclose(bdp_file);
		fclose(bch_file);
		return false;
	}

	arena_base = malloc(ARENA_SIZE);
	if (!arena_base) _asset_fatal("Arena allocation failed");
	arena_pos = 0;

	pthread_mutex_init(&file_lock, NULL);

	for (unsigned int cluster = 0; cluster < bch_header->cluster_count; ++cluster) {
		cluster_state_register[cluster].loaded = 0;
		cluster_state_register[cluster].loading = 0;
		cluster_state_register[cluster].load_done = 0;
		cluster_state_register[cluster].load_thread = 0;
	}
	debug_print("[%s] Initialized asset system\n", ASSET_LOADER_SIGN);
	return true;
}

bool asset_find(const char *path, AssetHandle *output)
{
	char *norm_path = normalize_path(path);
	XXH64_hash_t target_hash = XXH64(norm_path, strlen(norm_path), BINARY_HASH_SEED);
	free(norm_path);

	int low = 0, high = bdp_header->asset_count - 1;
	while (low <= high) {
		int mid = low + (high - low) / 2;
		if (asset_register[mid].hash == target_hash) {
			output->asset_index = (uint32_t)mid;
			return true;
		}
		if (asset_register[mid].hash < target_hash) low = mid + 1;
		else high = mid - 1;
	}
	debug_print("[%s] Asset with vpath %s and hash %I64u not found\n", ASSET_LOADER_SIGN, path, target_hash);
	return false;
}

bool asset_load_cluster(const char *cluster_name)
{
	char *norm_path = normalize_path(cluster_name);
	XXH64_hash_t target_hash = XXH64(norm_path, strlen(norm_path), BINARY_HASH_SEED);
	free(norm_path);

	int32_t cluster_idx = cluster_find(target_hash);
	if (cluster_idx < 0) {
		debug_print("[%s] Cluster not found\n", ASSET_LOADER_SIGN);
		return false;
	}

	if (cluster_state_register[cluster_idx].loaded) return true;

	if (__sync_bool_compare_and_swap(&cluster_state_register[cluster_idx].loading, 0, 1)) {
		cluster_state_register[cluster_idx].load_done = 0;
		int res = pthread_create(&cluster_state_register[cluster_idx].load_thread, NULL, load_cluster_thread, (void*)(intptr_t)cluster_idx);
		if (res != 0) {
			cluster_state_register[cluster_idx].loading = 0;
			debug_print("[%s] Failed to start cluster load thread\n", ASSET_LOADER_SIGN);
			return false;
		}
		return true;
	}
	return true;
}

bool asset_load_raw(AssetHandle handle, AssetBuffer buffer, size_t *bytes_written)
{
	const BdpEntry *entry = &asset_register[handle.asset_index];
	if (buffer.capacity < entry->raw_size) {
		debug_print("[%s] Insufficient buffer capacity\n", ASSET_LOADER_SIGN);
		return false;
	}

	XXH64_hash_t cluster_hash = entry->cluster_hash;
	int32_t cluster_idx = cluster_find(cluster_hash);
	if (cluster_idx < 0) {
		debug_print("[%s] Cluster not found\n", ASSET_LOADER_SIGN);
		return false;
	}

	if (!cluster_state_register[cluster_idx].loaded) {
		while (cluster_state_register[cluster_idx].loading) {
			sched_yield();
		}
		if (!cluster_state_register[cluster_idx].load_done) {
			debug_print("[%s] Cluster load failed or not done\n", ASSET_LOADER_SIGN);
			return false;
		}
	}

	uint32_t chunk_count = entry->chunk_count;
	size_t write_offset = 0;
	int32_t chunk_proc_start = find_chunk(entry->first_chunk);
	if (chunk_proc_start < 0) {
		debug_print("[%s] Failed to find first chunk of an asset in register\n", ASSET_LOADER_SIGN);
		return false;
	}

	if (buffer.capacity < write_offset + entry->raw_size) {
		debug_print("[%s] Buffer capacity is insufficient for asset loading\n", ASSET_LOADER_SIGN);
		return false;
	}

	for (uint32_t chunk_cursor = 0; chunk_cursor < chunk_count; ++chunk_cursor) {
		uint32_t chunk_proc = chunk_proc_start + chunk_cursor;
		CompressedChunk *scanned_chunk = &chunk_register[chunk_proc];
		int decompressed = LZ4_decompress_safe(
			(const char *)scanned_chunk->compressed_data,
			buffer.dest + write_offset,
			scanned_chunk->compressed_size,
			scanned_chunk->raw_size
		);
		if (decompressed < 0) {
			debug_print("[%s] LZ4 asset decompression for asset failed in chunk %u\n", ASSET_LOADER_SIGN, chunk_proc);
			return false;
		}
		write_offset += (size_t)decompressed;
	}

	if (bytes_written) *bytes_written = write_offset;
	return true;
}

void asset_unload_cluster(const char *cluster_name)
{
	char *norm_path = normalize_path(cluster_name);
	XXH64_hash_t target_hash = XXH64(norm_path, strlen(norm_path), BINARY_HASH_SEED);
	free(norm_path);

	int32_t cluster_idx = cluster_find(target_hash);
	if (cluster_idx < 0) {
		debug_print("[%s] Cluster not found\n", ASSET_LOADER_SIGN);
		return;
	}

	if (!cluster_state_register[cluster_idx].loaded) {
		debug_print("[%s] Attempting unload a Cluster already unloaded\n", ASSET_LOADER_SIGN);
		return;
	}

	if (cluster_state_register[cluster_idx].loading) {
		pthread_join(cluster_state_register[cluster_idx].load_thread, NULL);
	}

	cluster_state_register[cluster_idx].loaded = 0;
	BchEntry *cluster = &cluster_register[cluster_idx];

	int32_t proc_start = find_chunk(cluster->initial_chunk);
	if (proc_start < 0) {
		debug_print("[%s] Failed to locate first allocated chunk corresponding to cluster, unable to unload cluster\n", ASSET_LOADER_SIGN);
		return;
	}

	memmove(
		&chunk_register[proc_start],
		&chunk_register[proc_start + cluster->chunk_count],
		sizeof(CompressedChunk) * (loaded_chunks - (proc_start + cluster->chunk_count))
	);
	loaded_chunks -= cluster->chunk_count;
	debug_print("[%s] Unloaded cluster\n", ASSET_LOADER_SIGN);
}

void asset_system_shutdown(void)
{
	for (uint32_t i = 0; i < bch_header->cluster_count; ++i) {
		if (cluster_state_register[i].loading) {
			pthread_join(cluster_state_register[i].load_thread, NULL);
		}
	}

	pthread_mutex_destroy(&arena_lock);
	pthread_mutex_destroy(&file_lock);
	free(arena_base);
	free(chunk_register);
	free(cluster_state_register);
	free(bdp_header);
	free(bch_header);
	if (bdp_file) fclose(bdp_file);
	if (bch_file) fclose(bch_file);
	free(asset_register);
	free(cluster_register);
	debug_print("[%s] System has been shutdown\n", ASSET_LOADER_SIGN);
}

Image asset_retrieve_image(const char *vpath)
{
	AssetHandle handle;
	if(!asset_find(vpath, &handle)) return (Image){0};

	void *mem = malloc(asset_register[handle.asset_index].raw_size);
	AssetBuffer buff = {
		.dest = mem,
		.capacity = mem ? (uint32_t)asset_register[handle.asset_index].raw_size : 0
	};

	if (!buff.dest) {
		debug_print("[%s] Failed to allocate memory for asset loading\n", ASSET_LOADER_SIGN);
		return (Image){0};
	}

	size_t written;
	asset_load_raw(handle, buff, &written);

	char *file_ext = retrieve_file_ext(vpath);
	Image asset_img = LoadImageFromMemory(file_ext, buff.dest, written);
	if (!IsImageValid(asset_img)) {
		debug_print("[%s] Failed to load image from memory\n", ASSET_LOADER_SIGN);
		free(mem);
		return (Image){0};
	}

	free(mem);
	debug_print("[%s] Image %s loaded successfully\n", ASSET_LOADER_SIGN, vpath);
	return asset_img;

}

Texture2D asset_retrieve_texture(const char *vpath)
{
	Image asset_temp_img = asset_retrieve_image(vpath);

	if (!IsImageValid(asset_temp_img)) {
		debug_print("[%s] Failed to load image from memory\n", ASSET_LOADER_SIGN);
		return (Texture2D){0};
	}

	Texture2D asset_texture = LoadTextureFromImage(asset_temp_img);
	free(asset_temp_img.data);

	if (!IsTextureValid(asset_texture)) {
		debug_print("[%s] Failed to load texture from image\n", ASSET_LOADER_SIGN);
		return (Texture2D){0};
	}

	debug_print("[%s] Texture %s loaded successfully\n", ASSET_LOADER_SIGN, vpath);
	return asset_texture;
}
