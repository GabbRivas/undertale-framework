#ifndef WINDOW_CONTROLLER_H
#define WINDOW_CONTROLLER_H

#include <raylib.h>
#include <stdint.h>

typedef struct
{
	const char	*border_path;
	Color 		color;
	float		alpha;
	Texture2D	border_texture;
} WindowBorder;

typedef struct
{
	uint64_t 	base_width;
	uint64_t 	base_height;

	uint64_t	virtual_width;
	uint64_t	virtual_height;

	bool		border_enabled;

	RenderTexture2D	window_buffer;
	RenderTexture2D	game_buffer;

	WindowBorder	border;
} WindowControlData;

extern WindowControlData* window_control;

// Ideally, when transitioning into a new border we'd need to draw the new border over the old one, and fade in the new border.
// To achieve that, instead of storing into an array the borders we simply allow the user to draw over other borders if needed.

void game_draw_begin(void);
void game_draw_end(void);

//This allows to (if needed, draw onto the application buffer once the gamebuffer has been drawn into the application buffer) in between game_draw_end and game_draw_update
void game_draw_update(void);

void game_draw_shutdown(void);
void toggle_borderless_fullscreen(void);

#endif //_WINDOW_CONTROLLER_H
