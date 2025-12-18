#ifndef _INPUT_SYSTEM_H
#define _INPUT_SYSTEM_H

// Made by Gab Rivas 15/12/25
#include <stdint.h>
#include <raylib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_INPUT_BINDS	3

typedef struct {
	uint16_t keybinds[MAX_INPUT_BINDS];
	uint8_t bounded_keybinds;

	uint16_t mousebinds[MAX_INPUT_BINDS];
	uint8_t bounded_mousebinds;

	uint16_t gamepad_binds[MAX_INPUT_BINDS];
	uint8_t gamepad_device_binds[MAX_INPUT_BINDS];
	uint8_t bounded_gamepad_binds;
} InputRegister;

typedef enum {
	INPUT_TYPE_KEYBOARD,
	INPUT_TYPE_MOUSE,
	INPUT_TYPE_GAMEPAD
} InputType;

typedef enum {
	INPUT_IDLE,
	INPUT_HELD,
	INPUT_PRESSED,
	INPUT_RELEASED
} InputState;

typedef enum {
	INPUT_ACCEPT, INPUT_CANCEL,
	INPUT_LEFT, INPUT_RIGHT, INPUT_UP, INPUT_DOWN,
	INPUT_PROC_END
} Input;

void input_init();
bool input_bind(Input input, uint8_t slot, InputType type, uint8_t device, uint16_t input_code);

void input_refresh();

bool is_input_held(Input input);
bool is_input_pressed(Input input);
bool is_input_released(Input input);

#endif /* _INPUT_SYSTEM_H */
