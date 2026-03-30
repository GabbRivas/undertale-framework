#include <text/text_definitions.h>
#include <text/text_system.h>
#include <text/text_tokenizer.h>
#include <text/text_internal.h>
#include <debug.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static TextObject* 		text_object_pool[MAX_TEXT_OBJECTS];
static TokenCmdlet 		registered_commands[TOKEN_PROC_END] = {0};
uint32_t			command_list_len = 0;
uint32_t			next_free_slot = 0;

static inline int compare_cmdlets(const void *cmdlet_a, const void *cmdlet_b)
{
	return ((TokenCmdlet*)cmdlet_a)->hash < ((TokenCmdlet*)cmdlet_b)->hash ? -1 : 1;
}

void text_system_init(void)
{
	TokenCmdlet cmdlets[] = {
		{"pos x", TOKEN_POSITION_X, 0},
		{"pos y", TOKEN_POSITION_Y, 0},
		{"align", TOKEN_ALIGNMENT, 0},

		{"endln", TOKEN_NEW_LINE, 0},
		{"instant", TOKEN_INSTANT_TOGGLE, 0},
		{"speed", TOKEN_SPEED, 0},
		{"voice", TOKEN_VOICE, 0},
		{"sleep", TOKEN_SLEEP, 0},
		{"char progression", TOKEN_CHAR_PROGRESSION, 0},

		{"color", TOKEN_COLOR, 0},
		{"color ext", TOKEN_COLOR_EXT, 0},
		{"shake", TOKEN_SHAKE, 0},
		{"wave", TOKEN_WAVE, 0},
		{"rainbow", TOKEN_RAINBOW, 0},

		{"scale x", TOKEN_SCALE_X, 0},
		{"scale y", TOKEN_SCALE_Y, 0},
	};

	for (uint32_t proc = 0; proc < (uint32_t)TOKEN_PROC_END; ++proc)
	{
		cmdlets[proc].hash = XXH64(cmdlets[proc].name, strlen(cmdlets[proc].name), 0);
		registered_commands[proc] = cmdlets[proc];
	}

	qsort(registered_commands, TOKEN_PROC_END, sizeof(TokenCmdlet), compare_cmdlets);

	for (uint32_t proc = 0; proc < (uint32_t)TOKEN_PROC_END; ++proc)
	{
		debug_print("%s Registered command [%s] in position [%u] with hash [%llu]\n", TEXT_SYSTEM_SIGN, registered_commands[proc].name, proc, registered_commands[proc].hash);
	}
}

static inline void find_next_free_slot(void)
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

uint32_t text_create_object(void)
{
	uint32_t idx = next_free_slot;
	text_object_pool[idx] = calloc(1, sizeof(TextObject));
	if(!text_object_pool[idx])
	{
		debug_print("%s Failed to allocate memory for text object\n", TEXT_SYSTEM_SIGN);
		return -1;
	}

	debug_print("%s Created text object with [IDX %u]\n", TEXT_SYSTEM_SIGN, idx);
	find_next_free_slot();
	return idx;
}

bool text_set_string_target(uint32_t object_idx, const char *text, ...)
{
	TextObject *txt_obj = text_object_pool[object_idx];
	if (!txt_obj) return false;

	size_t len = strlen(text) + 1;
	if (len > txt_obj->template_capacity)
	{
		char *p = realloc(txt_obj->format_template, len);
		if (!p) return false;
		txt_obj->format_template = p;
		txt_obj->template_capacity = len;
	}
	memcpy(txt_obj->format_template, text, len);

	va_list args;
	va_start(args, text);
	bool ret_val = _resolve_template(txt_obj, args);
	va_end(args);
	if (!ret_val) return false;

	_parse_template(txt_obj, registered_commands);
	return true;
}

void text_mark_live(uint32_t object_idx, bool live)
{
	TextObject *txt_obj = text_object_pool[object_idx];
	if (!txt_obj) return;
	txt_obj->live_state = live;
}

bool text_live_update_string(uint32_t object_idx, ...)
{
	TextObject *txt_obj = text_object_pool[object_idx];
	if (!txt_obj || !txt_obj->live_state) return false;

	va_list args;
	va_start(args, object_idx);

	bool ret_val = _resolve_template(txt_obj, args);
	va_end(args);

	if (!ret_val) return false;

	_parse_template(txt_obj, registered_commands);
	return true;
}

bool text_destroy_object(uint32_t idx)
{
	if(!text_object_pool[idx]) return true;

	TextObject *txt_obj = text_object_pool[idx];
	free(txt_obj->format_template);
	free(txt_obj->resolved_text);
	free(txt_obj->tokens);
	free(txt_obj);
	text_object_pool[idx] = NULL;

	debug_print((!text_object_pool[idx] ? "%s Destroyed text object with [IDX %u]\n" : "%s Failed to destroy text object with [IDX %u]\n"), TEXT_SYSTEM_SIGN, idx);
	if (!text_object_pool[idx]) next_free_slot = idx < next_free_slot ? idx : next_free_slot;
	return true;
}
