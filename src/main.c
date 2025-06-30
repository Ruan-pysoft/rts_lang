#include "ministd_fmt.h"
#include "ministd_io.h"
#include "ministd_string.h"

const char ref src =
	"[ 'a 'b 'c -> 'c 'a 'b : rot rot ] := unrot\n"
	"[ int -> ' :\n"
	"  0 1\n"
	"  [ int int int -> ' bool : rot 0 > swp drop ]\n"
	"  [ int int int -> ' : unrot dup rot + ]\n"
	"  while\n"
	"] := fib\n"
	"10 fib\n"
;

typedef struct pos_t {
	usz offset;
	usz line;
	usz line_pos;
} pos_t;

typedef struct tok_t {
	pos_t pos;
	usz len;
} tok_t;

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

void
tok_display(const char ref src, tok_t ref token, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;
	usz written = 0;

	while (written < token->len) {
		isz bytes_written = write(
			file, (ptr)(src + token->pos.offset + written),
			token->len - written, &err
		);
		TRY_VOID(err);
		if (bytes_written == 0) ERR_VOID(ERR_IO);

		written += bytes_written;
	}
}

tok_t
parse_tok(const char ref src, pos_t ref pos)
{
	tok_t res;

	res.pos.offset = 0;
	res.pos.line = 0;
	res.pos.line_pos = 0;
	res.len = 0;

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

int
main(void)
{
	tok_t token;
	pos_t pos = (pos_t) {
		.offset = 0,
		.line = 1,
		.line_pos = 1,
	};

	fputs(src, stdout, NULL);
	puts("===", NULL);

	for (;;) {
		token = parse_tok(src, &pos);

		if (token.len == 0) break;

		tok_display(src, &token, stdout, NULL);
		prints(" @ ", NULL);
		printi(token.pos.line, NULL);
		printc(',', NULL);
		printi(token.pos.line_pos, NULL);
		putc('\n', NULL);
	}

	return 0;
}
