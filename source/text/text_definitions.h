#ifndef TEXT_DEFINITIONS_H
#define TEXT_DEFINITIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>
#include <xxhash/xxhash.h>
#include <text/text_tokenizer.h>

#define MAX_TEXT_OBJECTS	64

typedef struct
{
	char 		*format_template;
	size_t		template_capacity;

	char 		*resolved_text;
	size_t		resolved_text_capacity;

	ParsedToken 	*tokens;
	uint32_t	token_count;
	uint32_t	token_capacity;

	uint32_t	visible_length;
	uint32_t	character_progression;

	bool		live_state;
	bool		instant;
	bool		skippable;

	float		character_time;
	float		character_speed;

	void		*glyph_data;
} TextObject;

typedef struct
{
	uint32_t	codepoint;

	float 		x;
	float		y;

	float		scale_x;
	float		scale_y;
} TextGlyph;

#endif // TEXT_DEFINITIONS_H
