#include <raylib.h>
#include <debug.h>
#include <asset_loader/asset_loader.h>
#include <input/input.h>
#include <input/input_definitions.h>
#include <window_controller/window_controller.h>
#include <window_controller/window_border.h>
#include <window_controller/window_drawer.h>
#include <window_controller/window_definitions.h>

#define DATA_FILE "data0"

#ifdef DEBUG
	FILE* debug_log_pointer = NULL;
#endif

static inline void input_custom(void)
{
	input_bind(INPUT_ACCEPT, 0, INPUT_TYPE_KEYBOARD, 0, KEY_Z);
	input_bind(INPUT_ACCEPT, 1, INPUT_TYPE_KEYBOARD, 0, KEY_ENTER);

	input_bind(INPUT_CANCEL, 0, INPUT_TYPE_KEYBOARD, 0, KEY_X);
	input_bind(INPUT_CANCEL, 1, INPUT_TYPE_KEYBOARD, 0, KEY_LEFT_SHIFT);
	input_bind(INPUT_CANCEL, 2, INPUT_TYPE_KEYBOARD, 0, KEY_RIGHT_SHIFT);

	input_bind(INPUT_LEFT, 0, INPUT_TYPE_KEYBOARD, 0, KEY_LEFT);
	input_bind(INPUT_LEFT, 1, INPUT_TYPE_KEYBOARD, 0, KEY_A);

	input_bind(INPUT_RIGHT, 0, INPUT_TYPE_KEYBOARD, 0, KEY_RIGHT);
	input_bind(INPUT_RIGHT, 1, INPUT_TYPE_KEYBOARD, 0, KEY_D);

	input_bind(INPUT_UP, 0, INPUT_TYPE_KEYBOARD, 0, KEY_UP);
	input_bind(INPUT_UP, 1, INPUT_TYPE_KEYBOARD, 0, KEY_W);

	input_bind(INPUT_DOWN, 0, INPUT_TYPE_KEYBOARD, 0, KEY_DOWN);
	input_bind(INPUT_DOWN, 1, INPUT_TYPE_KEYBOARD, 0, KEY_S);

	input_bind(INPUT_FULLSCREEN, 0, INPUT_TYPE_KEYBOARD, 0, KEY_F4);
	input_bind(INPUT_FULLSCREEN, 1, INPUT_TYPE_KEYBOARD, 0, KEY_F);
}

int main(void)
{

	#ifdef DEBUG
		debug_log_pointer = fopen("debug.log", "w");
	#endif

	debug_print("[MAIN] Initialized application\n");
	asset_system_init(DATA_FILE);
	init_window_control(640, 480);
	input_custom();

	WindowControlData* window_data = get_window_control_data();

	asset_load_cluster(ASSET_LOADER_BORDER_CLUSTER);
	WindowBorder simple_border = window_new_border("border_cluster/border_story.png");
	asset_unload_cluster(ASSET_LOADER_BORDER_CLUSTER);
	//Holy compression the 278KB bdp by just loading the border cluster goes from ~57MB RAM up to ~100MB RAM
	window_set_current_border(&simple_border);
	window_enable_border();

	asset_load_cluster(ASSET_LOADER_GLOBAL_CLUSTER);
	InitWindow(window_data->virtual_width, window_data->virtual_height, "Undertale Engine");

	Font ut_font = asset_retrieve_font("global_cluster/8bitoperator_jve.ttf");

	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
	game_draw_init();

	while (!WindowShouldClose())
	{
		input_refresh();
		if (is_input_pressed(INPUT_FULLSCREEN)) toggle_borderless_fullscreen();

		game_draw_begin();
			ClearBackground(BLACK);
			DrawTextEx(ut_font, TextFormat("FPS (%i)\nFrame-Temps (%f)ms", GetFPS(), GetFrameTime()), (Vector2){32,32}, 32, 1, WHITE);
		game_draw_end();

		game_draw_update();
	}

	// Clean-Up
	CloseWindow();
	asset_system_shutdown();
	game_draw_shutdown();

	#ifdef DEBUG
		fclose(debug_log_pointer);
	#endif
}
