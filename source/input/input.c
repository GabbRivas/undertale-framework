#include <input/input.h>
#include <input/input_definitions.h>
#include <debug.h>
#include <input/input_internal.h>

static InputSlot	input_register[INPUT_COUNT];
static bool 	 	input_prev_down[INPUT_COUNT] = {false};
static InputState	input_state[INPUT_COUNT] = {INPUT_IDLE};


bool input_bind(Input input, uint8_t slot, InputType type, uint8_t device, uint16_t input_code)
{
	if ((unsigned)input >= INPUT_COUNT || slot >= MAX_INPUT_BINDS)
	{
		debug_print((unsigned)input >= INPUT_COUNT ?
			"%s Trying to assign an input out of range, [Input: %i] in range %i, [Slot: %i] in range %i\n" :
			"%s Attempting to assign an input out of range, [Input: %i] in range %i, [Slot: %i] in range %i\n",
			INPUT_SYSTEM_SIGN, input, INPUT_COUNT, slot, MAX_INPUT_BINDS);
		return false;
	}

	switch (type)
	{
		case INPUT_TYPE_KEYBOARD:
		_bind_keyboard_input(&input_register[input], slot, input_code);
		break;
		case INPUT_TYPE_MOUSE:
		_bind_mouse_input(&input_register[input], slot, input_code);
		break;
		case INPUT_TYPE_GAMEPAD:
		_bind_gamepad_input(&input_register[input], device, slot, input_code);
		break;
		default:
		return false;
	}

	debug_print("%s Successfully bound [Input %i] to [Slot %i] with [Type %i] and [Device %i]\n",INPUT_SYSTEM_SIGN, input, slot, type, device);
	return true;
}

void input_refresh(void)
{
	for (int input_index = 0; input_index < INPUT_COUNT; ++input_index)
	{
		bool down = _is_input_down(&input_register[input_index]);

		if(down) input_state[input_index] = input_prev_down[input_index] ? INPUT_HELD : INPUT_PRESSED;
		else input_state[input_index] = input_prev_down[input_index] ? INPUT_RELEASED : INPUT_IDLE;

		input_prev_down[input_index] = down;
	}
}

bool is_input_held(Input input)
{
	return input_state[input] == INPUT_HELD || input_state[input] == INPUT_PRESSED;
}

bool is_input_pressed(Input input)
{
	return input_state[input] == INPUT_PRESSED;
}

bool is_input_released(Input input)
{
	return input_state[input] == INPUT_RELEASED;
}
