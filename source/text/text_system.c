#include <text/text_system.h>
#include <raylib.h>
#include <stdint.h>
#include <text/text_tokenizer.h>
#include <debug.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <xxhash/xxhash.h>

static TextObject* 		text_object_pool[MAX_TEXT_OBJECTS];
static TokenCmdlet 		registered_commands[TOKEN_PROC_END] = {0};
uint32_t			command_list_len = 0;
uint32_t			next_free_slot = 0;

void text_system_init(void)
{
	TokenCmdlet cmdlets[] = {
		{"pos_x", TOKEN_POSITION_X},
		{"pos_y", TOKEN_POSITION_Y},
		{"align", TOKEN_ALIGNMENT},

		{"endln", TOKEN_NEW_LINE},
		{"instant", TOKEN_INSTANT_TOGGLE},
		{"speed", TOKEN_SPEED},
		{"voice", TOKEN_VOICE},
		{"sleep", TOKEN_SLEEP},
		{"char_progression", TOKEN_CHAR_PROGRESSION},

		{"color", TOKEN_COLOR},
		{"color_ext", TOKEN_COLOR_EXT},
		{"shake", TOKEN_SHAKE},
		{"wave", TOKEN_WAVE},
		{"rainbow", TOKEN_RAINBOW},

		{"scale_x", TOKEN_SCALE_X},
		{"scale_y", TOKEN_SCALE_Y},
	};

	for (uint32_t proc = 0; proc < (uint32_t)TOKEN_PROC_END; ++proc)
	{
		XXH64_hash_t hash = XXH64(cmdlets[proc].name, strlen(cmdlets[proc].name), 0);
		XXH64_hash_t len = hash % TOKEN_PROC_END;
		registered_commands[len] = cmdlets[proc];
		debug_print("%s Registered command [%s] in position [%i]\n", TEXT_SYSTEM_SIGN, cmdlets[proc].name, len);
		command_list_len = proc+1;
		//In case collisions exist, simply modify the array size and the hash modulo.
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

static bool _format_text(TextObject *text_obj, const char *text, ...)
{
	va_list args, args_copy;
	va_start(args, text);

	va_copy(args_copy, args);

	int required_size = vsnprintf(NULL, 0, text, args_copy);

	if (required_size < 0)
	{
		va_end(args_copy);
		va_end(args);
		return false;
	}

	char *new_ptr = realloc(text_obj->text, required_size + 1);
	if (!new_ptr)
	{
		va_end(args_copy);
		va_end(args);
		return false;
	}
	text_obj->text = new_ptr;

	int status_code = vsnprintf(text_obj->text, required_size + 1, text, args_copy);

	va_end(args_copy);
	va_end(args);

	debug_print("%s Formated text into text object with [Status Code %d]\n", TEXT_SYSTEM_SIGN, status_code);
	return status_code >= 0;
}

static void _parse_text(TextObject *text_obj)
{
	//O(n) time complexity, should probably look forward to improve it.
	char *cursor = text_obj->text;

	unsigned int proc = 0;
	unsigned int start = -1;

	while (*cursor)
	{
		if(*cursor == TEXT_LEFT_DELIMITER)
		{
			start = proc;
		}

		if(*cursor == TEXT_RIGHT_DELIMITER && start != -1)
		{
			_validate_command(text_obj, start, proc-start);
			start = -1;
		}
		++cursor;
		++proc;
	};
}

static void _validate_command(TextObject *text_obj, unsigned int start, unsigned int len)
{
	bool negation_cmd = false;
	const char *name_start = text_obj->text + start + 1;
	uint32_t name_len = len;

	if (*name_start == '/')
	{
		negation_cmd = true;
		++name_start;
		--name_len;
	}

	uint32_t real_len = 0;
	while (real_len < name_len && name_start[real_len] != TEXT_TOKEN_DELIMITER && name_start[real_len] != TEXT_RIGHT_DELIMITER)
	{
		++real_len;
	}

	XXH64_hash_t hash = XXH64(name_start, real_len, 0);
	uint32_t pos = hash % command_list_len;

	TokenCmdlet cmd = registered_commands[pos];
	if (cmd.name != NULL && strncmp(cmd.name, name_start, real_len) == 0)
	{
		debug_print("%s Found command [%s] in position [%i]\n", TEXT_SYSTEM_SIGN, cmd.name, *(name_start) - start - 1);
	}
	else
	{
		return;
	}
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

	_parse_text(text_object_pool[object_idx]);

	return state;
}
