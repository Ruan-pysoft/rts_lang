#include "typecheck.h"

#include "ministd.h"
#include "ministd_fmt.h"
#include "ministd_memory.h"

#include "lex.h"
#include "types.h"

bool
exec_transform(type_stack_t ref type_stack, const transform_t ref transform)
{
	if (type_stack->len < transform->from_len) return false;

	generic_map_t genmap;
	generic_map_init(&genmap, transform->n_generics);

	for (usz i = 0; i < transform->from_len; ++i) {
		const type_t ref stack_type = &type_stack->stack[
			type_stack->len - transform->from_len + i
		];
		const type_t ref trans_type = &transform->from[i];

		if (trans_type->type == TT_GENERIC && trans_type->t.gen == genmap.len) {
			generic_map_add(&genmap, stack_type);
			continue;
		} else if (trans_type->type == TT_GENERIC && trans_type->t.gen < genmap.len) {
			trans_type = &genmap.types[trans_type->t.gen];
		} else if (trans_type->type == TT_GENERIC && trans_type->t.gen > genmap.len) {
			/* shouldn't happen ever */

			fprints("Something went horribly wrong while applying transform input types", stderr, 0);
			exit(1);
		}

		if (trans_type->type != stack_type->type) return false;
		switch (trans_type->type) {
			case TT_SIMPLE: {
				if (trans_type->t.simple != stack_type->t.simple) {
					return false;
				}
			break; }
			case TT_GENERIC: {
				if (trans_type->t.gen != stack_type->t.gen) {
					return false;
				}
			break; }
			case TT_TRANSFORM: {
				/* Not possible for now, TODO: deal with this later */
				exit(1);
			break; }
		}
	}

	for (usz i = 0; i < transform->from_len; ++i) {
		type_stack_pop(type_stack);
	}

	for (usz i = 0; i < transform->to_len; ++i) {
		const type_t ref trans_type = &transform->to[i];

		if (trans_type->type == TT_GENERIC) {
			if (trans_type->t.gen >= genmap.len) {
				/* shouldn't happen ever */

				fprints("Something went horribly wrong while applying transform output types", stderr, 0);
				exit(1);
			}

			trans_type = &genmap.types[trans_type->t.gen];
		}

		type_stack_push(type_stack, trans_type);
	}

	return true;
}

bool
exec_type(type_stack_t ref type_stack, const type_t ref type)
{
	switch (type->type) {
		case TT_SIMPLE: {
			type_stack_push(type_stack, type);
			return true;
		break; }
		case TT_GENERIC: {
			type_stack_push(type_stack, type);
			return true;
		break; }
		case TT_TRANSFORM: {
			return exec_transform(type_stack, &type->t.trans);
		break; }
	}

	return false;
}

bool
stackspec_to_transform(const stackspec_t ref stackspec, const char ref src,
		       transform_t ref trans_out)
{
	transform_init(trans_out);

	char own own generics = alloc(sizeof(char own) * stackspec->in_len, NULL);

	trans_out->from_len = stackspec->in_len;
	trans_out->from = alloc(sizeof(type_t) * trans_out->from_len, NULL);
	for (usz i = 0; i < stackspec->in_len; ++i) {
		if (src[stackspec->in_types[i].pos.offset] == '\'') {
			for (usz j = 0; j < trans_out->n_generics; ++j) {
				if (match(src, &stackspec->in_types[i], generics[j])) {
					type_init(&trans_out->from[i]);
					trans_out->from[i].type = TT_GENERIC;
					trans_out->from[i].t.gen = j;
					goto found_generic;
				}
			}

			generics[trans_out->n_generics] = alloc(stackspec->in_types[i].len + 1, NULL);
			memmove(generics[trans_out->n_generics], src + stackspec->in_types[i].pos.offset, stackspec->in_types[i].len);
			generics[trans_out->n_generics][stackspec->in_types[i].len] = 0;
			trans_out->from[i].type = TT_GENERIC;
			trans_out->from[i].t.gen = trans_out->n_generics;
			++trans_out->n_generics;

		found_generic:;
		} else {
			type_init(&trans_out->from[i]);
			if (match(src, &stackspec->in_types[i], "int")) {
				trans_out->from[i].t.simple = ST_INT;
			} else if (match(src, &stackspec->in_types[i], "bool")) {
				trans_out->from[i].t.simple = ST_BOOL;
			} else {
				fprints("Invalid type!", stderr, NULL);
				exit(1);
			}
		}
	}

	for (usz i = 0; i < stackspec->out_len; ++i) {
		if (match(src, &stackspec->out_types[i], "'")) {
			trans_out->to_len += trans_out->from_len;
		} else {
			++trans_out->to_len;
		}
	}
	trans_out->to = alloc(sizeof(type_t) * trans_out->to_len, NULL);
	usz trans_i = 0;
	for (usz i = 0; i < stackspec->out_len; ++i) {
		if (match(src, &stackspec->out_types[i], "'")) {
			for (usz j = 0; j < trans_out->from_len; ++j) {
				type_copy(&trans_out->to[trans_i + j], &trans_out->from[j]);
			}
			trans_i += trans_out->from_len;
		} else if (src[stackspec->out_types[i].pos.offset] == '\'') {
				usz j = 0;
				for (; j < trans_out->n_generics; ++j) {
					if (match(src, &stackspec->out_types[i], generics[j])) {
						type_init(&trans_out->to[trans_i]);
						trans_out->to[trans_i].type = TT_GENERIC;
						trans_out->to[trans_i].t.gen = j;
						break;
					}
				}

				if (j == trans_out->n_generics) {
					/* generic not in input types */
					fprints("Invalid generic!", stderr, NULL);
					exit(1);
				}

			++trans_i;
		} else {
			type_init(&trans_out->to[trans_i]);
			if (match(src, &stackspec->out_types[i], "int")) {
				trans_out->to[trans_i].t.simple = ST_INT;
			} else if (match(src, &stackspec->out_types[i], "bool")) {
				trans_out->to[trans_i].t.simple = ST_BOOL;
			} else {
				fprints("Invalid type!", stderr, NULL);
				exit(1);
			}
			++trans_i;
		}
	}

	return true;
}
bool
resolve_type(type_t ref type_out, const item_t ref item, const char ref src,
	     const type_defs_t ref type_defs)
{
	switch (item->type) {
		case IT_NULL: {
			return false;
		break; }
		case IT_ASSGN: {
			/* isn't supposed to happen anyways */
			return false;
		break; }
		case IT_WORD: {
			const type_t ref looked_up = type_defs_lookup_word(
				type_defs, &item->item.word, src
			);
			if (looked_up == NULL) return false;
			type_copy(type_out, looked_up);
			return true;
		break; }
		case IT_BLOCK: {
			type_init(type_out);
			type_out->type = TT_TRANSFORM;
			return stackspec_to_transform(
				&item->item.block.stackspec, src,
				&type_out->t.trans
			);
		break; }
	}
	return false;
}

void
type_stack_start(type_stack_t ref type_stack_out,
		 const transform_t ref transform)
{
	type_stack_init(type_stack_out);
	type_stack_out->len = transform->from_len;
	if (type_stack_out->len > type_stack_out->cap) {
		type_stack_out->cap = type_stack_out->len;
		type_stack_out->stack = realloc(
			type_stack_out->stack,
			sizeof(type_t) * type_stack_out->cap,
			NULL
		);
	}
	for (usz i = 0; i < transform->from_len; ++i) {
		type_copy(&type_stack_out->stack[i], &transform->from[i]);
	}
}

bool
type_stack_check(const type_stack_t ref type_stack,
		 const transform_t ref transform)
{
	if (type_stack->len != transform->to_len) return false;

	for (usz i = 0; i < transform->to_len; ++i) {
		const type_t ref stack_type = &type_stack->stack[i];
		const type_t ref trans_type = &transform->to[i];

		if (stack_type->type != trans_type->type) {
			return false;
		}

		switch (stack_type->type) {
			case TT_SIMPLE: {
				if (stack_type->t.simple != trans_type->t.simple) {
					return false;
				}
			break; }
			case TT_GENERIC: {
				if (stack_type->t.gen != trans_type->t.gen) {
					return false;
				}
			break; }
			case TT_TRANSFORM: {
				/* Not possible for now, TODO: deal with this later */
				exit(1);
			break; }
		}
	}

	return true;
}

bool
apply_item(type_stack_t ref type_stack, const item_t ref item,
	   const char ref src, const type_defs_t ref type_defs)
{
	if (item->type == IT_ASSGN) {
		return false;
	}

	type_t item_type;
	if (!resolve_type(&item_type, item, src, type_defs)) return false;

	if (item->type == IT_WORD) {
		bool res = exec_type(type_stack, &item_type);
		type_deinit(&item_type);

		return res;
	} else {
		type_stack_push(type_stack, &item_type);
		type_deinit(&item_type);

		return true;
	}
}

bool
apply_item_toplevel(type_stack_t ref type_stack, const item_t ref item,
		    const char ref src, type_defs_t ref type_defs)
{
	if (item->type == IT_ASSGN) {
		if (type_stack->len == 0) {
			return false;
		}
		if (type_stack_peek(type_stack)->type == TT_GENERIC) {
			return false;
		}

		type_defs_add_word(
			type_defs, &item->item.assgn.word, src,
			type_stack_peek(type_stack)
		);
		type_stack_pop(type_stack);

		return true;
	}

	type_t item_type;
	if (!resolve_type(&item_type, item, src, type_defs)) return false;

	if (item->type == IT_WORD) {
		bool res = exec_type(type_stack, &item_type);
		type_deinit(&item_type);

		return res;
	} else {
		type_stack_push(type_stack, &item_type);
		type_deinit(&item_type);

		return true;
	}
}

bool
check_block(const block_t ref block, const char ref src,
	    const type_defs_t ref type_defs)
{
	type_stack_t type_stack;
	transform_t trans;

	if (!stackspec_to_transform(&block->stackspec, src, &trans)) return false;
	type_stack_start(&type_stack, &trans);

	for (usz i = 0; i < block->len; ++i) {
		if (!apply_item(&type_stack, &block->items[i], src, type_defs)) {
			type_stack_deinit(&type_stack);
			transform_deinit(&trans);

			return false;
		}
	}

	bool res = type_stack_check(&type_stack, &trans);
	type_stack_deinit(&type_stack);
	transform_deinit(&trans);

	return res;
}
