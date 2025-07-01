#include "lex.h"

#include "ministd_string.h"

bool
match(const char ref src, tok_t ref token, const char ref with)
{
	const usz len = strnlen(with, token->len+1);

	if (len != token->len) return false;

	for (usz i = 0; i < len; ++i) {
		if (src[token->pos.offset + i] != with[i]) return false;
	}

	return true;
}

tok_t
parse_tok(const char ref src, pos_t ref pos)
{
	tok_t res;

	tok_init(&res);

	while (src[pos->offset] != 0 && src[pos->offset] <= ' ') {
		if (src[pos->offset] == '\n') {
			pos->line++;
			pos->line_pos = 0;
		}
		pos->offset++;
		pos->line_pos++;
	}

	res.pos = *pos;

	while (src[res.pos.offset + res.len] != 0 && src[res.pos.offset + res.len] > ' ') {
		res.len++;
		pos->offset++;
		pos->line_pos++;
	}

	return res;
}
