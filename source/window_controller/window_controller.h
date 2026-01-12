#ifndef WINDOW_CONTROLLER_H
#define WINDOW_CONTROLLER_H

#include <raylib.h>
#include <stdint.h>
#include <window_controller/window_definitions.h>

#define 	WINDOW_CONTROLLER_SIGNATURE	"WINDOW CONTROLLER"

// Ideally, when transitioning into a new border we'd need to draw the new border over the old one, and fade in the new border.
// To achieve that, instead of storing into an array the borders we simply allow the user to draw over other borders if needed.

void window_center(void);

void init_window_control(uint64_t _base_width, uint64_t _base_height);
WindowControlData* get_window_control_data(void);

void toggle_borderless_fullscreen(void);

#endif //_WINDOW_CONTROLLER_H
