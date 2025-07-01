#include "ministd_fmt.h"
#include "ministd_memory.h"

#include "lex.h"
#include "parse.h"
#include "types.h"

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

int
main(void)
{
	tok_t token;
	pos_t pos = (pos_t) {
		.offset = 0,
		.line = 1,
		.line_pos = 1,
	};
	tok_t own toks = alloc(sizeof(tok_t) * 1024, NULL);
	usz toks_cap = 1024;
	usz toks_len = 0;

	fputs(src, stdout, NULL);
	puts("===", NULL);

	for (;;) {
		token = parse_tok(src, &pos);

		if (token.len == 0) break;

		if (toks_len == toks_cap) {
			toks_cap *= 2;
			toks = realloc(toks, sizeof(tok_t) * toks_cap, NULL);
		}
		toks[toks_len++] = token;

		tok_display(&token, src, stdout, NULL);
		prints(" @ ", NULL);
		printi(token.pos.line, NULL);
		printc(',', NULL);
		printi(token.pos.line_pos, NULL);
		putc('\n', NULL);
	}
	puts("===", NULL);

	usz toks_pos = 0;
	item_t item;

	while (toks_pos < toks_len) {
		item = parse_item(src, toks, toks_len, &toks_pos, NULL);

		item_display(&item, src, stdout, NULL);
		putc('\n', NULL);
	}

	return 0;
}
