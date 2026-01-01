#include <raylib.h>
#include <debug.h>
#include <asset_loader/asset_loader.h>

#define DATA_FILE "data0"

#ifdef DEBUG
	FILE* debug_log_pointer = NULL;
#endif

int main(void)
{

	#ifdef DEBUG
		debug_log_pointer = fopen("debug.log", "w");
	#endif

	debug_print("[MAIN] Initialized application\n");
	asset_system_init(DATA_FILE);
	InitWindow(960, 540, "Undertale Engine");
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

	asset_load_cluster(ASSET_LOADER_GLOBAL_CLUSTER);

	while (!WindowShouldClose())
	{
		// TODO

		BeginDrawing();
			ClearBackground(BLACK);
		EndDrawing();
	}

	CloseWindow();
	asset_system_shutdown();

	#ifdef DEBUG
		fclose(debug_log_pointer);
	#endif
}
