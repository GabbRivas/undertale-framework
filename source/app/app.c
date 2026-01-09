#include <raylib.h>
#include <debug.h>
#include <asset_loader/asset_loader.h>
#include <input/input.h>
#include <input/input_definitions.h>

#define DATA_FILE "data0"

#ifdef DEBUG
	FILE* debug_log_pointer = NULL;
#endif

static inline void input_custom(void)
{
	input_bind(INPUT_ACCEPT, 0, INPUT_TYPE_KEYBOARD, 0, KEY_Z);
	input_bind(INPUT_ACCEPT, 0, INPUT_TYPE_KEYBOARD, 0, KEY_ENTER);

	input_bind(INPUT_CANCEL, 0, INPUT_TYPE_KEYBOARD, 0, KEY_X);
	input_bind(INPUT_CANCEL, 0, INPUT_TYPE_KEYBOARD, 0, KEY_LEFT_SHIFT);
	input_bind(INPUT_CANCEL, 0, INPUT_TYPE_KEYBOARD, 0, KEY_RIGHT_SHIFT);

	input_bind(INPUT_LEFT, 0, INPUT_TYPE_KEYBOARD, 0, KEY_LEFT);
	input_bind(INPUT_LEFT, 0, INPUT_TYPE_KEYBOARD, 0, KEY_A);

	input_bind(INPUT_RIGHT, 0, INPUT_TYPE_KEYBOARD, 0, KEY_RIGHT);
	input_bind(INPUT_RIGHT, 0, INPUT_TYPE_KEYBOARD, 0, KEY_D);

	input_bind(INPUT_UP, 0, INPUT_TYPE_KEYBOARD, 0, KEY_UP);
	input_bind(INPUT_UP, 0, INPUT_TYPE_KEYBOARD, 0, KEY_W);

	input_bind(INPUT_DOWN, 0, INPUT_TYPE_KEYBOARD, 0, KEY_DOWN);
	input_bind(INPUT_DOWN, 0, INPUT_TYPE_KEYBOARD, 0, KEY_S);

	input_bind(INPUT_FULLSCREEN, 0, INPUT_TYPE_KEYBOARD, 0, KEY_F4);
	input_bind(INPUT_FULLSCREEN, 0, INPUT_TYPE_KEYBOARD, 0, KEY_F);
}

int main(void)
{

	#ifdef DEBUG
		debug_log_pointer = fopen("debug.log", "w");
	#endif

	debug_print("[MAIN] Initialized application\n");
	asset_system_init(DATA_FILE);
	input_custom();

	InitWindow(960, 540, "Undertale Engine");
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

	asset_load_cluster(ASSET_LOADER_GLOBAL_CLUSTER);

	while (!WindowShouldClose())
	{
		input_refresh();
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
