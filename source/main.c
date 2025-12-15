#include <raylib.h>

int main(void){

	// Pre-Create

	InitWindow(640, 480, "Undertale Engine");
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

	// Init

	while (!WindowShouldClose()) {
		BeginDrawing();
		EndDrawing();
	}

	// Cleanup

	CloseWindow();
}
