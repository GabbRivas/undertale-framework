#include <text/text_system.h>
#include <raylib.h>
#include <stdint.h>
#include <text/text_tokenizer.h>
#include <debug.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static TextObject* 		text_object_pool[MAX_TEXT_OBJECTS];
uint32_t			next_free_slot = 0;

static inline void find_next_free_slot()
{
	if (next_free_slot+1 < MAX_TEXT_OBJECTS && !text_object_pool[next_free_slot+1])
	{
		next_free_slot++;
		return;
	}

	for (uint32_t slot = 0; slot < MAX_TEXT_OBJECTS; ++slot)
	{
		if (!text_object_pool[slot])
		{
			next_free_slot = slot;
			return;
		}
	}
}

static bool _format_text(TextObject *text_obj, const char *text, ...)
{
	va_list args;
	va_start(args, text);
	int status_code = vsprintf(text_obj->text, text, args);
	va_end(args);
	debug_print("%s Formated text into text object with [Status Code %d]\n", TEXT_SYSTEM_SIGN, status_code);
	return status_code >= 0;
}

static bool _parse_text(TextObject *text_obj, const char*text)
{
	return true;
}

uint32_t text_create_object(void)
{
	uint32_t idx = next_free_slot;
	text_object_pool[idx] = malloc(sizeof(TextObject));
	if(!text_object_pool[idx])
	{
		debug_print("%s Failed to allocate memory for text object\n", TEXT_SYSTEM_SIGN);
		return -1;
	}

	debug_print("%s Created text object with [IDX %lu]\n", TEXT_SYSTEM_SIGN);
	find_next_free_slot();
	return idx;
}

bool text_destroy_object(uint32_t idx)
{
	if(!text_object_pool[idx]) return true;

	free(text_object_pool[idx]);
	text_object_pool[idx] = NULL;

	debug_print((!text_object_pool[idx] ? "%s Destroyed text object with [IDX %lu]\n" : "%s Failed to destroy text object with [IDX %lu]\n"), TEXT_SYSTEM_SIGN, idx);
	if (!text_object_pool[idx]) next_free_slot = idx < next_free_slot ? idx : next_free_slot;
	return !text_object_pool[idx];
}

bool text_set_string_target(uint32_t object_idx, const char *text, ...)
{
	if (!text_object_pool[object_idx]) return false;

	bool state = true;
	va_list args;
	va_start(args, text);
	state = _format_text(text_object_pool[object_idx], text, args);
	va_end(args);

	if (!state) return state;

	state = _parse_text(text_object_pool[object_idx], text_object_pool[object_idx]->text);

	return state;
}
