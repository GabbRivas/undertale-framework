#ifndef WINDOW_DEFINITIONS_H
#define WINDOW_DEFINITIONS_H

#include <stdint.h>
#include <raylib.h>

typedef struct
{
	Image		border_image;
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

	WindowBorder*	border;
} WindowControlData;

#endif
