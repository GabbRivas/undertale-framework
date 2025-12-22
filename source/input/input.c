#include "input.h"

static InputRegister _input_container[INPUT_COUNT];

static bool _input_prev_down[INPUT_COUNT];
static InputState _input_state_register[INPUT_COUNT] = {INPUT_IDLE};

static inline bool _is_input_down(const InputRegister *_input){
	for (int _entry = 0; _entry < _input->bounded_keybinds; ++_entry){
		if (IsKeyDown(_input->keybinds[_entry])) return true;
	}
	for (int _entry = 0; _entry < _input->bounded_mousebinds; ++_entry){
		if (IsMouseButtonDown(_input->mousebinds[_entry])) return true;
	}
	for (int _entry = 0; _entry < _input->bounded_gamepad_binds; ++_entry){
		if (IsGamepadAvailable(_input->gamepad_device_binds[_entry]) && IsGamepadButtonDown(_input->gamepad_device_binds[_entry], _input->gamepad_binds[_entry])) return true;
	}

	return false;
}

static inline void _bind_keyboard_input(InputRegister* _target, uint8_t _slot, uint16_t _input_code){
	_target->keybinds[_slot] = _input_code;
	if (_slot >= _target->bounded_keybinds) _target->bounded_keybinds = _slot+1;
}

static inline void _bind_mouse_input(InputRegister* _target, uint8_t _slot, uint16_t _input_code){
	_target->mousebinds[_slot] = _input_code;
	if (_slot >= _target->bounded_mousebinds) _target->bounded_mousebinds = _slot+1;
}

static inline void _bind_gamepad_input(InputRegister* _target, uint8_t _slot, uint8_t _device, uint16_t _input_code){
	_target->gamepad_device_binds[_slot] = _device;
	_target->gamepad_binds[_slot] = _input_code;
	if (_slot >= _target->bounded_gamepad_binds) _target->bounded_gamepad_binds = _slot+1;
}

bool input_bind(Input input, uint8_t slot, InputType type, uint8_t device, uint16_t input_code){
	if ((unsigned)input >= INPUT_COUNT || slot >= MAX_INPUT_BINDS) return false;

	switch (type) {
		case INPUT_TYPE_KEYBOARD:
		_bind_keyboard_input(&_input_container[input], slot, input_code);
		break;
		case INPUT_TYPE_MOUSE:
		_bind_mouse_input(&_input_container[input], slot, input_code);
		break;
		case INPUT_TYPE_GAMEPAD:
		_bind_gamepad_input(&_input_container[input],slot, device, input_code);
		break;
		default:
		return false;
	}
	return true;
}

void input_refresh(){
	for (int _input_index = 0; _input_index < INPUT_COUNT; ++_input_index){
		const InputRegister *_loaded_input = &_input_container[_input_index];
		bool down = _is_input_down(_loaded_input);

		if (down) _input_state_register[_input_index] = _input_prev_down[_input_index] ?  INPUT_HELD :  INPUT_PRESSED;
		else _input_state_register[_input_index] = _input_prev_down[_input_index] ? INPUT_RELEASED : INPUT_IDLE;

		_input_prev_down[_input_index] = down;

	}
}

bool is_input_held(Input input){
	if ((unsigned)input >= INPUT_COUNT) return false;
	return (_input_state_register[input] == INPUT_HELD || _input_state_register[input] == INPUT_PRESSED);
}

bool is_input_pressed(Input input){
	if ((unsigned)input >= INPUT_COUNT) return false;
	return _input_state_register[input] == INPUT_PRESSED;
}
bool is_input_released(Input input){
	if ((unsigned)input >= INPUT_COUNT) return false;
	return _input_state_register[input] == INPUT_RELEASED;
}
