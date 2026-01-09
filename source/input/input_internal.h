#ifndef INPUT_INTERNAL_H
#define INPUT_INTERNAL_H

#include <input/input_definitions.h>

bool _is_input_down(const InputSlot *_input);

void _bind_keyboard_input(InputSlot *_target, uint8_t _slot, uint16_t _input_code);
void _bind_mouse_input(InputSlot *_target, uint8_t _slot, uint16_t _input_code);
void _bind_gamepad_input(InputSlot *_target, uint8_t _device, uint8_t _slot, uint16_t _input_code);

#endif // _INPUT_INTERNAL_H
