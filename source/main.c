#include <raylib.h>
#include "input/input.h"
#include "window_control/window_control.h"

static void input_custom(void){
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

int main(void){

	// Pre-Create
	input_custom();

	// Create

	set_border_path("resources/textures/game_border/1727.png");
	enable_border();

	InitWindow(get_window_width(), get_window_height(), "Undertale Engine");
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

	// Init

	draw_init();
	// Loop
	while (!WindowShouldClose()) {
		input_refresh();

		if (is_input_pressed(INPUT_FULLSCREEN)) toggle_borderless_fullscreen();

		draw_begin();{
		}
		draw_end();
	}

	// Cleanup

	draw_un_init();
	CloseWindow();
}
