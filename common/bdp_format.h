#ifndef BDP_FORMAT_H
#define BDP_FORMAT_H

#include <stdint.h>
#include <xxhash/xxhash.h>

// Made by Gab Rivas 22/12/25


#define PACKER_VERSION			0x10000099
#define BINARY_HASH_SEED		0

#define BDP_FILE_EXT			".bdp"
#define BCH_FILE_EXT			".bct"

#define BDP_MAGIC_BYTES 		((char[4]){'B', 'D', 'P', '1'})

#define BCH_MAGIC_BYTES 		((char[4]){'B', 'C', 'H', '1'})

#define BDP_CHUNK_RAW_SIZE 		65536
#define BDP_CHUNK_NAME_LENGTH		64

#define BDP_CLUSTER_NAME_LENGTH		32
#define BCH_TRIGGER			"cluster"

#define BDP_FLAG_COMPRESSED_LZ4 	(1 << 0)

// BDP v1.1

typedef enum
{
	ASSET_TYPE_TEXTURE,
	ASSET_TYPE_SOUND,
	ASSET_TYPE_FONT,
	ASSET_TYPE_INVALID,
} AssetType;


typedef struct
{
	char 		magic[4];
	uint32_t 	version;
	uint32_t 	asset_count;
	uint32_t 	toc_offset;
	uint32_t 	data_offset;
	uint32_t 	chunk_register_offset;
} BdpHeader;

typedef struct
{
	XXH64_hash_t 	hash;
	AssetType 	type;
	uint32_t	first_chunk;
	uint32_t	chunk_count;
	XXH64_hash_t	cluster_hash;
	uint32_t 	raw_size;
} BdpEntry;

typedef struct
{
	uint32_t 	data_offset;
	uint32_t 	size;
	uint32_t 	raw_size;
} BdpChunk;

typedef struct
{
	char 		magic[4];
	uint32_t	version;
	uint32_t	cluster_count;
} BchHeader;

typedef struct
{
	XXH64_hash_t	hash;
	uint32_t	initial_chunk;
	uint32_t	chunk_count;
} BchEntry;

#endif // BDP_FORMAT_H
