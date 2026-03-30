#include <text/text_definitions.h>
#include <text/text_internal.h>
#include <text/text_tokenizer.h>
#include <stdio.h>
#include <debug.h>
#include <stdlib.h>
#include <string.h>

bool _resolve_template(TextObject *object, va_list args)
{
	va_list args_copy;
	va_copy(args_copy, args);

	int required = vsnprintf(NULL, 0, object->format_template, args_copy);
	va_end(args_copy);

	if (required < 0)
	{
		debug_print("%s Failed to fetch necessary mem-size for text\n",TEXT_INTERNAL_DEBUG, required);
		return false;
	}

	if ((size_t)required+1 > object->resolved_text_capacity)
	{
		char *p = realloc(object->resolved_text, (size_t)required+1);
		if (!p)
		{
			debug_print("%s Failed to realloc resolved text buffer\n", TEXT_INTERNAL_DEBUG, required);
			return false;
		}
		object->resolved_text = p;
		object->resolved_text_capacity = (size_t)required + 1;
	}

	va_copy(args_copy, args);
	int written = vsnprintf(object->resolved_text, object->resolved_text_capacity, object->format_template, args_copy);
	debug_print("%s Resolved text: [%s] with [%i length]\n", TEXT_INTERNAL_DEBUG, object->resolved_text, written);
	va_end(args_copy);
	return written >= 0;
}

static void _push_token(TextObject *object, ParsedToken token)
{
	if (object->token_count >= object->token_capacity)
	{
		uint32_t new_capacity = object->token_capacity == 0 ? 8 : object->token_capacity * 2;
		ParsedToken *tkn = realloc(object->tokens, new_capacity * sizeof(ParsedToken));
		if (!tkn)
		{
			debug_print("%s Failed to realloc token buffer\n", TEXT_INTERNAL_DEBUG);
			return;
		}

		object->tokens = tkn;
		object->token_capacity = new_capacity;
	}
	object->tokens[object->token_count++] = token;
}

void _parse_template(TextObject *object, TokenCmdlet* cmd_array)
{
	// TODO: Minimize branching
	object->token_count = 0;

	const char *raw = object->resolved_text;
	uint32_t idx = 0;
	uint32_t visible_chars = 0;

	while (raw[idx] != '\0')
	{
		if (raw[idx] == TEXT_LEFT_DELIMITER)
		{
			uint32_t end = idx + 1;
			while (raw[end] != '\0' && raw[end] != TEXT_RIGHT_DELIMITER) end++;

			if (raw[end] == '\0')
			{
				++idx;
				continue;
			}

			const char *cmd_start = raw + idx + 1;
			bool negation = false;
			if (*cmd_start == '/')
			{
				negation = true;
				++cmd_start;
			}

			uint32_t cmd_len = 0;
			while (cmd_start[cmd_len] != TEXT_TOKEN_DELIMITER && cmd_start[cmd_len] != TEXT_RIGHT_DELIMITER && cmd_start[cmd_len] != '\0') ++cmd_len;

			uint32_t trimmed = cmd_len;
			while (trimmed > 0 && cmd_start[cmd_len - 1] == ' ') --trimmed;

			const char *args_start = NULL;
			uint32_t args_len = 0;
			if (cmd_start[cmd_len] == TEXT_TOKEN_DELIMITER)
			{
				args_start = cmd_start + cmd_len + 1;
				while (*args_start == ' ') ++args_start;
				const char *args_end = raw + end;
				while (args_end > args_start && (*args_end - 1) == ' ') --args_end;
				args_len = (uint32_t)(args_end - args_start);
			}

			_validate_command(cmd_array, object, cmd_start, trimmed, negation, visible_chars, args_start, args_len);

			idx = end + 1;
			continue;
		}

		++idx;
		++visible_chars;
	}

	object->visible_length = visible_chars;
	debug_print("%s Parsed template: [%u visible chars], [%u tokens]\n", TEXT_INTERNAL_DEBUG, visible_chars, object->token_count);
}

bool _validate_command(TokenCmdlet* cmd_array, TextObject *object, const char *cmd_start, uint32_t cmd_len, bool negation, uint32_t visible_pos, const char *args_start, uint32_t args_len)
{
	XXH64_hash_t hash = XXH64(cmd_start, cmd_len, 0);

	int low = 0, high = TOKEN_PROC_END - 1;
	TokenCmdlet *found = NULL;
	while (low <= high)
	{
		int mid = low + (high - low) /2;
		if(cmd_array[mid].hash == hash)
		{
			found = &cmd_array[mid];
			break;
		}
		if (cmd_array[mid].hash < hash) low = mid + 1;
		else high = mid - 1;
	}

	if (!found)
	{
		debug_print("%s Command not found: [%.*s] [Hash %llu]\n", TEXT_INTERNAL_DEBUG, (int)cmd_len, cmd_start, hash);
		return false;
	}

	ParsedToken token = {
		.type = found->type,
		.position = visible_pos,
		.negative = negation,
	};

	if (args_start && args_len > 0)
	{
		uint32_t copy_len = args_len < sizeof(token.args) - 1 ? args_len : sizeof(token.args) - 1;
		memcpy(token.args, args_start, copy_len);
		token.args[copy_len] = '\0';
	}

	_push_token(object, token);
	debug_print("%s Pushed token [%s] at visible char %u [Neg: %d, Args: %s]", TEXT_INTERNAL_DEBUG, found->name, visible_pos, negation, token_args);
	return true;
}
