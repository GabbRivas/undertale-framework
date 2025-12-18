#include <raylib.h>
#include "input/input.h"

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
}

int main(void){

	// Pre-Create
	input_custom();

	// Create

	InitWindow(640, 480, "Undertale Engine");
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

	// Init

	// Loop
	while (!WindowShouldClose()) {
		input_refresh();

		BeginDrawing();
		EndDrawing();
	}

	// Cleanup

	CloseWindow();
}
