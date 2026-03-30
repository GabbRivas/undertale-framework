#include <limits.h>
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

static bool _validate_command(TextObject *text_obj, unsigned int start, unsigned int len)
{
	bool negation_cmd = false;
	const char *cmd_start = text_obj->text + start + 1;

	if (*cmd_start == '/')
	{
		negation_cmd = true;
		++cmd_start;
		--len;
	}

	uint32_t cmd_len = 0;
	while (cmd_len < len && cmd_start[cmd_len] != TEXT_TOKEN_DELIMITER && cmd_start[cmd_len] != TEXT_RIGHT_DELIMITER) ++cmd_len;

	XXH64_hash_t target_hash = XXH64(cmd_start, cmd_len, 0);

	int infimum = 0;
	int supremum = TOKEN_PROC_END - 1;
	TokenCmdlet *found_cmd = NULL;

	while (infimum <= supremum)
	{
		int mid = infimum + (supremum - infimum)/2;

		if (registered_commands[mid].hash == target_hash)
		{
			found_cmd = &registered_commands[mid];
			break;
		}

		if (registered_commands[mid].hash < target_hash)
		{
			infimum = mid + 1;
		}
		else
		{
			supremum = mid - 1;
		}
	}

	if (found_cmd != NULL)
	{
		debug_print("%s Text parser found command: [%s] (Negation: %d)\n", TEXT_SYSTEM_SIGN, found_cmd->name, negation_cmd);



		return true;
	}

	debug_print("%s Text parser could not find given command\n", TEXT_SYSTEM_SIGN);
	return false;
}

static void _parse_text(TextObject *text_obj)
{
	// O(n) time complexity, should aspire to make it O(nln(n)) if possible [still pretty good]
	char *raw_text = text_obj->text;

	unsigned int idx = 0;
	unsigned int visible_chars = 0;

	text_obj->token_count = 0;

	while (raw_text[idx] != '\0')
	{
		if (raw_text[idx] == TEXT_LEFT_DELIMITER)
		{
			uint32_t end = idx + 1;
			bool is_valid = false;

			while (raw_text[end] != TEXT_RIGHT_DELIMITER && raw_text[end] != '\0') ++end;

			is_valid = _validate_command(text_obj, idx, end - idx);
			if (is_valid)
			{
				idx = end + 1;
				continue;
			}
		}

		++visible_chars;
		++idx;
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

bool text_destroy_object(uint32_t idx)
{
	if(!text_object_pool[idx]) return true;

	free(text_object_pool[idx]);
	text_object_pool[idx] = NULL;

	debug_print((!text_object_pool[idx] ? "%s Destroyed text object with [IDX %u]\n" : "%s Failed to destroy text object with [IDX %u]\n"), TEXT_SYSTEM_SIGN, idx);
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
