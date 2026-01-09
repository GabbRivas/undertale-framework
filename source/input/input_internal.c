#include <input/input_internal.h>
#include <input/input_definitions.h>
#include <raylib.h>

bool _is_input_down(const InputSlot *_input)
{
	for (int entry = 0; entry < _input->bounded_keybinds; ++entry) if (IsKeyDown(_input->keybinds[entry])) return true;
	for (int entry = 0; entry < _input->bounded_mousebinds; ++entry) if (IsMouseButtonDown(_input->bounded_mousebinds)) return true;
	for (int entry = 0; entry < _input->bounded_keybinds; ++entry) if (IsGamepadButtonDown(_input->gamepad_devicebinds[entry], _input->gamepadbinds[entry])) return true;
	return false;
}

void _bind_keyboard_input(InputSlot *_target, uint8_t _slot, uint16_t _input_code)
{
	_target->keybinds[_slot] = _input_code;
	if (_slot >= _target->bounded_keybinds) _target->bounded_keybinds = _slot + 1;
}

void _bind_mouse_input(InputSlot *_target, uint8_t _slot, uint16_t _input_code)
{
	_target->mousebinds[_slot] = _input_code;
	if (_slot >= _target->bounded_mousebinds) _target->bounded_mousebinds = _slot + 1;
}

void _bind_gamepad_input(InputSlot *_target, uint8_t _device, uint8_t _slot, uint16_t _input_code)
{
	_target->gamepad_devicebinds[_slot] = _device;
	_target->gamepadbinds[_slot] = _input_code;
	if (_slot >= _target->bounded_gamepadbinds) _target->bounded_gamepadbinds = _slot + 1;
}
