#ifndef TEXT_TOKENIZER_H
#define TEXT_TOKENIZER_H

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>

#define TEXT_LEFT_DELIMITER "<"
#define TEXT_RIGHT_DELIMITER ">"

typedef enum
{
	TOKEN_POSITION,
	TOKEN_ALIGNMENT,

	TOKEN_TEXT,
	TOKEN_NEW_LINE,
	TOKEN_INSTANT_TOGGLE,
	TOKEN_SPEED,
	TOKEN_VOICE,
	TOKEN_SLEEP,
	TOKEN_CHAR_PROGRESSION,

	TOKEN_COLOR,
	TOKEN_COLOR_EXT, //Four colors, top, left, up, down.
	TOKEN_SHAKE,
	TOKEN_WAVE,
	TOKEN_RAINBOW,

	TOKEN_SCALE_X,
	TOKEN_SCALE_Y
}TokenType;

typedef struct
{
	TokenType type;
	union
	{
		const char 	*text;
		Color 		Color;
		float 		fvalue;
		struct 		{ float x, y; } vec2;
		struct 		{ Color top_left, top_right, down_left, down_right; } gradient;
	};
} Token;

#endif // TEXT_TOKENIZER_H
