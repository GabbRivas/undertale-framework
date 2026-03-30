#ifndef TEXT_SYSTEM_H
#define TEXT_SYSTEM_H

#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <text/text_tokenizer.h>

#ifdef DEBUG
#define TEXT_SYSTEM_SIGN 		"TEXT_SYSTEM"
#else
#define TEXT_SYSTEM_SIGN		""
#endif

void 		text_system_init(void); //create at the beginning of the program the lookup tables for commands.

uint32_t	text_create_object(void);

bool		text_set_string_target(uint32_t object_idx, const char *text, ...);
void 		text_mark_live(uint32_t object_idx, bool live);
bool		text_live_update_string(uint32_t object_idx, ...);

bool		text_destroy_object(uint32_t idx);

void 		text_draw_update(void);

#endif // TEXT_SYSTEM_H
