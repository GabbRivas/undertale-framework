// Made by Gab Rivas 14/12/25
#ifndef _WINDOW_CONTROL_H
#define _WINDOW_CONTROL_H

#include <raylib.h>
#include <stdint.h>
#include <_mingw_mac.h>
#include <_mingw_off_t.h>
#include <math.h>

void set_border_path(const char* path);
void enable_border(void);

void draw_init(void);

void draw_begin(void);
void draw_end(void);

void draw_un_init(void);

unsigned int get_window_width(void);
unsigned int get_window_height(void);

void toggle_borderless_fullscreen(void);

bool is_border_active(void);

#endif /* _WINDOW_CONTROL_H */
