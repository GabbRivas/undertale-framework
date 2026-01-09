#ifndef INPUT_DEFINITIONS_H
#define INPUT_DEFINITIONS_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_INPUT_BINDS 3

typedef struct
{
	unsigned int 	keybinds[MAX_INPUT_BINDS];
	uint8_t		bounded_keybinds;

	unsigned int 	mousebinds[MAX_INPUT_BINDS];
	uint8_t		bounded_mousebinds;

	unsigned int	gamepadbinds[MAX_INPUT_BINDS];
	uint8_t		gamepad_devicebinds[MAX_INPUT_BINDS];
	uint8_t		bounded_gamepadbinds;
} InputSlot;

typedef enum
{
	INPUT_TYPE_KEYBOARD,
	INPUT_TYPE_MOUSE,
	INPUT_TYPE_GAMEPAD
} InputType;

typedef enum
{
	INPUT_IDLE,
	INPUT_HELD,
	INPUT_PRESSED,
	INPUT_RELEASED
} InputState;

typedef enum
{
	INPUT_ACCEPT, INPUT_CANCEL,
	INPUT_LEFT, INPUT_RIGHT, INPUT_UP, INPUT_DOWN,
	INPUT_FULLSCREEN,
	INPUT_COUNT
} Input;

#endif // INPUT_DEFINITIONS_H
