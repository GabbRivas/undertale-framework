#ifndef WINDOW_BORDER_H
#define WINDOW_BORDER_H

#include <window_controller/window_controller.h>
#include <raylib.h>

WindowBorder window_new_border(const char *path);
bool window_init_border(WindowBorder* border);
bool window_set_current_border(WindowBorder* border);

void window_draw_border(WindowBorder border);

void window_enable_border(void);
void window_disable_border(void);
bool is_border_enabled();


#endif 	// WINDOW_BORDER_H
