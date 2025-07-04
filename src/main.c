#include "ministd_fmt.h"
#include "ministd_memory.h"

#include "lex.h"
#include "parse.h"
#include "typecheck.h"
#include "types.h"

const char ref src =
	"[ 'a 'b 'c -> 'c 'a 'b : rot rot ] := unrot\n"
	"[ int -> ' :\n"
	"  0 1\n" /* $0 a b */
	"  [ int int int -> ' bool : rot 0 > swp drop ]\n" /* a b $0 cond */
	"  [ int int int -> ' : 1 - unrot dup rot + ]\n" /* $0-1 b a+b */
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
	puts("===", NULL);

	flush(stdout, NULL);

	/* TODO: the code below causes a segfault, but I'm too tired to debug it rn */

	type_stack_t type_stack;
	type_defs_t type_defs;
	type_stack_init(&type_stack);
	type_defs_init(&type_defs);

	type_t rot_type;
	type_init(&rot_type);
	rot_type.type = TT_TRANSFORM;
	transform_init(&rot_type.t.trans);
	rot_type.t.trans.n_generics = 3;
	rot_type.t.trans.from_len = 3;
	rot_type.t.trans.from = alloc(sizeof(type_t) * 3, NULL);
	for (usz i = 0; i < 3; ++i) {
		type_init(&rot_type.t.trans.from[i]);
		rot_type.t.trans.from[i].type = TT_GENERIC;
		rot_type.t.trans.from[i].t.gen = i;
	}
	rot_type.t.trans.to_len = 3;
	rot_type.t.trans.to = alloc(sizeof(type_t) * 3, NULL);
	for (usz i = 0; i < 3; ++i) {
		type_init(&rot_type.t.trans.to[i]);
		rot_type.t.trans.to[i].type = TT_GENERIC;
		rot_type.t.trans.to[i].t.gen = (i+1)%3;
	}
	type_defs_add(&type_defs, "rot", &rot_type);

	type_t int_type;
	type_init(&int_type);
	int_type.t.simple = ST_INT;

	type_t bool_type;
	type_init(&bool_type);
	bool_type.t.simple = ST_BOOL;

	type_defs_add(&type_defs, "0", &int_type);
	type_defs_add(&type_defs, "1", &int_type);
	type_defs_add(&type_defs, "10", &int_type);

	type_t gt_type;
	type_init(&gt_type);
	gt_type.type = TT_TRANSFORM;
	transform_init(&gt_type.t.trans);
	gt_type.t.trans.n_generics = 0;
	gt_type.t.trans.from_len = 2;
	gt_type.t.trans.from = alloc(sizeof(type_t) * 2, NULL);
	for (usz i = 0; i < 2; ++i) {
		type_copy(&gt_type.t.trans.from[i], &int_type);
	}
	gt_type.t.trans.to_len = 3;
	gt_type.t.trans.to = alloc(sizeof(type_t) * 3, NULL);
	for (usz i = 0; i < 2; ++i) {
		type_copy(&gt_type.t.trans.to[i], &int_type);
	}
	type_copy(&gt_type.t.trans.to[2], &bool_type);
	type_defs_add(&type_defs, ">", &gt_type);

	type_t swp_type;
	type_init(&swp_type);
	swp_type.type = TT_TRANSFORM;
	transform_init(&swp_type.t.trans);
	swp_type.t.trans.n_generics = 2;
	swp_type.t.trans.from_len = 2;
	swp_type.t.trans.from = alloc(sizeof(type_t) * 2, NULL);
	for (usz i = 0; i < 2; ++i) {
		type_init(&swp_type.t.trans.from[i]);
		swp_type.t.trans.from[i].type = TT_GENERIC;
		swp_type.t.trans.from[i].t.gen = i;
	}
	swp_type.t.trans.to_len = 2;
	swp_type.t.trans.to = alloc(sizeof(type_t) * 2, NULL);
	for (usz i = 0; i < 2; ++i) {
		type_init(&swp_type.t.trans.to[i]);
		swp_type.t.trans.from[i].type = TT_GENERIC;
		swp_type.t.trans.to[i].t.gen = 1-i;
	}
	type_defs_add(&type_defs, "swp", &swp_type);

	type_t drop_type;
	type_init(&drop_type);
	drop_type.type = TT_TRANSFORM;
	transform_init(&drop_type.t.trans);
	drop_type.t.trans.n_generics = 1;
	drop_type.t.trans.from_len = 1;
	drop_type.t.trans.from = alloc(sizeof(type_t) * 1, NULL);
	type_init(&drop_type.t.trans.from[0]);
	drop_type.t.trans.from[0].type = TT_GENERIC;
	drop_type.t.trans.from[0].t.gen = 0;
	drop_type.t.trans.to_len = 0;
	drop_type.t.trans.to = NULL;
	type_defs_add(&type_defs, "drop", &drop_type);

	type_t add_type;
	type_init(&add_type);
	add_type.type = TT_TRANSFORM;
	transform_init(&add_type.t.trans);
	add_type.t.trans.n_generics = 0;
	add_type.t.trans.from_len = 2;
	add_type.t.trans.from = alloc(sizeof(type_t) * 2, NULL);
	for (usz i = 0; i < 2; ++i) {
		type_copy(&add_type.t.trans.from[i], &int_type);
	}
	add_type.t.trans.to_len = 1;
	add_type.t.trans.to = alloc(sizeof(type_t) * 1, NULL);
	type_copy(&add_type.t.trans.to[0], &int_type);
	type_defs_add(&type_defs, "+", &add_type);

	/* TODO: while type
	 * honestly, way too much effor for me to do rn, it's almost 11 pm
	 * Also I'll probably first make an easier way to define types...
	 */

	/* TODO: this should be done automatically with the assignment */
	type_t fib_type;
	type_init(&fib_type);
	fib_type.type = TT_TRANSFORM;
	transform_init(&fib_type.t.trans);
	fib_type.t.trans.n_generics = 0;
	fib_type.t.trans.from_len = 1;
	fib_type.t.trans.from = alloc(sizeof(type_t) * 1, NULL);
	type_copy(&fib_type.t.trans.from[0], &int_type);
	fib_type.t.trans.to_len = 1;
	fib_type.t.trans.to = alloc(sizeof(type_t) * 1, NULL);
	type_copy(&fib_type.t.trans.to[0], &int_type);
	type_defs_add(&type_defs, "fib", &fib_type);

	toks_pos = 0;

	while (toks_pos < toks_len) {
		type_t type;
		item = parse_item(src, toks, toks_len, &toks_pos, NULL); /* wait, am I leaking memory here? */
		if (!resolve_type(&type, &item, src, &type_defs)) {
			fputs("Error fetching type of ", stdout, NULL);
			item_display(&item, src, stdout, NULL);
			putc('\n', NULL);
			fputs("Checking: ", stdout, NULL);
			fprinti(apply_item_toplevel(&type_stack, &item, src, &type_defs), stdout, NULL);
			putc('\n', NULL);
		} else {
			fputs("Type check of ", stdout, NULL);
			item_display(&item, src, stdout, NULL);
			fputs(" of type ", stdout, NULL);
			type_display(&type, stdout, NULL);
			fputs(": ", stdout, NULL);
			fprinti(apply_item_toplevel(&type_stack, &item, src, &type_defs), stdout, NULL);
			putc('\n', NULL);
		}

		if (item.type == IT_BLOCK) {
			fputs("Checking if the body lines up: ", stdout, NULL);
			fprinti(check_block(&item.item.block, src, &type_defs), stdout, NULL);
			putc('\n', NULL);
		}

		fputs("Type stack is now: ", stdout, NULL);
		type_stack_display(&type_stack, src, stdout, NULL);
		putc('\n', NULL);

		flush(stdout, NULL);

		type_deinit(&type);
	}

	return 0;
}
