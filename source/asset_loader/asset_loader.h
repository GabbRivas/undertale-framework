#ifndef ASSET_LOADER_H
#define ASSET_LOADER_H
// Made by Gab Rivas 27/12/25

#include <corecrt.h>
#include <stdint.h>
#include <xxhash/xxhash.h>
#include <bdp_format.h>
#include <raylib.h>
#include <pthread.h>
#include <stdatomic.h>

#define ASSET_LOADER_BORDER_CLUSTER 	"border_cluster"
#define ASSET_LOADER_GLOBAL_CLUSTER 	"global_cluster"
#define ASSET_LOADER_BATTLE_CLUSTER 	"battle_cluster"
#define ASSET_LOADER_OVERWORLD_CLUSTER 	"overworld_cluster"

#ifdef DEBUG
#define ASSET_LOADER_SIGN 		"ASSET_LOADER"
#else
#define ASSET_LOADER_SIGN		""
#endif

typedef struct {
    uint8_t 	loaded;
    uint8_t 	loading;
    uint8_t 	load_done;
    pthread_t 	load_thread;
} ClusterState;


typedef struct {
    uint32_t 	asset_index;
} AssetHandle;

typedef struct {
    void 	*dest;
    size_t 	capacity;
} AssetBuffer;

typedef struct {
    uint32_t 	idx;
    uint8_t 	*compressed_data;
    size_t 	compressed_size;
    size_t 	raw_size;
} CompressedChunk;

bool asset_system_init(const char *pack_path);
bool asset_find(const char *path, AssetHandle *output);
bool asset_load_cluster(const char *cluster_name);  // Non-blocking start

bool asset_load_raw(AssetHandle handle, AssetBuffer buffer, size_t *bytes_written);

void asset_unload_cluster(const char *cluster_name);
void asset_system_shutdown(void);

//Raylib wrapper
Image asset_retrieve_image(const char *vpath);
Texture2D asset_retrieve_texture(const char *vpath);
Font asset_retrieve_font(const char *vpath);

#endif // ASSET_LOADER_H
