#ifndef TEXT_SYSTEM_H
#define TEXT_SYSTEM_H

#include <raylib.h>
#include <stdint.h>
#include <text/text_tokenizer.h>

#ifdef DEBUG
#define TEXT_SYSTEM_SIGN 		"TEXT_SYSTEM"
#else
#define TEXT_SYSTEM_SIGN		""
#endif

#define MAX_TEXT_OBJECTS	64

typedef struct
{
	char 		*text;

	Token 		*tokens;
	int		token_count;

	int		update_frequencty;
	int		visible_characters;

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

	uint8_t		effects;
	Color		color;
} TextGlyph;

static bool _format_text(TextObject *text_obj, const char *text, ...);
static bool _parse_text(TextObject *text_obj, const char*text);

uint32_t	text_create_object(void);
bool		text_destroy_object(uint32_t idx);

bool		text_set_string_target(uint32_t object_idx, const char *text, ...);

#endif // TEXT_SYSTEM_H
