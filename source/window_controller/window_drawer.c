#include "debug.h"
#include <window_controller/window_definitions.h>
#include <window_controller/window_drawer.h>
#include <window_controller/window_controller.h>
#include <window_controller/window_border.h>
#include <math.h>

void game_draw_init(void)
{
	WindowControlData* window_control = get_window_control_data();
	window_control->game_buffer = LoadRenderTexture(window_control->base_width, window_control->base_height);
	window_control->window_buffer = LoadRenderTexture(window_control->virtual_width, window_control->virtual_height);
	debug_print("[%s] Initialized app buffers\n", WINDOW_DRAWER_SIGNATURE);
}

void game_draw_begin(void)
{
	WindowControlData* window_control = get_window_control_data();
	BeginTextureMode(window_control->game_buffer);
}
void game_draw_end(void)
{
	WindowControlData* window_control = get_window_control_data();
	EndTextureMode();
	if (window_control->border_enabled) window_draw_border(window_control->border);
	BeginTextureMode(window_control->window_buffer);

	int _target_x = (window_control->window_buffer.texture.width - window_control->game_buffer.texture.width)/2;
	int _target_y = (window_control->window_buffer.texture.height - window_control->game_buffer.texture.height)/2;

	DrawTexturePro(window_control->game_buffer.texture,
		(Rectangle){0,0,window_control->game_buffer.texture.width, -window_control->game_buffer.texture.height},
		(Rectangle){_target_x,_target_y,window_control->game_buffer.texture.width,window_control->game_buffer.texture.height},
		(Vector2){0,0}, 0, WHITE);
	EndTextureMode();
}

void game_draw_update(void)
{
	BeginDrawing();
	ClearBackground(BLACK);

	WindowControlData* window_control = get_window_control_data();
	float _scale_factor = fminf((float)GetScreenWidth()/window_control->window_buffer.texture.width, (float)GetScreenHeight()/window_control->window_buffer.texture.height);

	float offset_x = (GetScreenWidth() - window_control->window_buffer.texture.width * _scale_factor)/2;
	float offset_y = (GetScreenHeight() - window_control->window_buffer.texture.height * _scale_factor)/2;

	DrawTexturePro(window_control->window_buffer.texture,
		(Rectangle){0,0,window_control->window_buffer.texture.width, -window_control->window_buffer.texture.height},
		(Rectangle){offset_x,offset_y,window_control->window_buffer.texture.width * _scale_factor, window_control->window_buffer.texture.height * _scale_factor},
		(Vector2){0,0}, 0, WHITE);
	EndDrawing();
}

void game_draw_shutdown(void)
{
	WindowControlData* window_control = get_window_control_data();
	UnloadRenderTexture(window_control->window_buffer);
	UnloadRenderTexture(window_control->game_buffer);
	debug_print("[%s] Module shutdown completed\n", WINDOW_DRAWER_SIGNATURE);
}
