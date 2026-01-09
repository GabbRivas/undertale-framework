#ifndef INPUT_H
#define INPUT_H

#include <input/input_definitions.h>

#ifdef DEBUG
#define INPUT_SYSTEM_SIGN 		"INPUT_SYSTEM"
#else
#define INPUT_SYSTEM_SIGN		""
#endif

bool input_bind(Input input, uint8_t slot, InputType type, uint8_t device, uint16_t input_code);

void input_refresh(void);

bool is_input_held(Input input);
bool is_input_pressed(Input input);
bool is_input_released(Input input);

#endif // INPUT_H
