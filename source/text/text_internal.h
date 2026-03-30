#ifndef TEXT_INTERNAL_H
#define TEXT_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>
#include <xxhash/xxhash.h>
#include <text/text_definitions.h>
#include <text/text_tokenizer.h>

#ifdef DEBUG
#define TEXT_INTERNAL_DEBUG "TEXT_INTERNAL"
#else
#define TEXT_INTERNAL_DEBUG ""
#endif

bool _resolve_template(TextObject *object, va_list args);
void _parse_template(TextObject *object, TokenCmdlet* cmd_array);
bool _validate_command(TokenCmdlet* cmd_array, TextObject *object, const char *cmd_start, uint32_t cmd_len, bool negation, uint32_t visible_pos, const char *args_start, uint32_t args_len);

#endif // TEXT_INTERNAL_H
