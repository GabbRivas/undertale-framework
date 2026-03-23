#ifndef TEXT_TOKENIZER_H
#define TEXT_TOKENIZER_H

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>
#include <xxhash/xxhash.h>

#define TEXT_LEFT_DELIMITER '<'
#define TEXT_RIGHT_DELIMITER '>'
#define TEXT_TOKEN_DELIMITER ':'

//Any tokentype added should be also added to text_system.c on the init function.
// If planning to implement it, put it after proc_end to avoid exploding everything up and after implementing it but it before proc_end, and on the init system.
typedef enum
{
	TOKEN_POSITION_X,
	TOKEN_POSITION_Y,
	TOKEN_ALIGNMENT,

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
	TOKEN_SCALE_Y,

	TOKEN_PROC_END,
}TokenType;

typedef struct
{
	const char *name;
	TokenType type;
	XXH64_hash_t hash;
} TokenCmdlet;

typedef struct
{
	TokenType type;
} Token;



#endif // TEXT_TOKENIZER_H
