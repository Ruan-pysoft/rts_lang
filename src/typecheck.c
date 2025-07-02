#include "typecheck.h"

#include "ministd.h"
#include "ministd_fmt.h"
#include "ministd_memory.h"

#include "lex.h"
#include "types.h"

type_t own
gen_assgn_type(void)
{
	type_t own res = alloc(sizeof(type_t), NULL);

	type_init(res);
	res->type = TT_TRANSFORM;

	transform_init(&res->t.transform);
	res->t.transform.n_generics = 1;
	res->t.transform.from_len = 1;
	res->t.transform.from = alloc(sizeof(typespec_t), NULL);
	typespec_init(res->t.transform.from);

	return res;
}
type_t own
copy_block_type(const block_t ref block, const char ref src,
		const type_defs_t ref type_defs)
{
	type_t own res = alloc(sizeof(type_t), NULL);

	type_init(res);
	res->type = TT_TRANSFORM;

	transform_init(&res->t.transform);
	res->t.transform.from_len = block->stackspec.in_len;
	res->t.transform.from = alloc(
		sizeof(typespec_t) * res->t.transform.from_len, NULL
	);

	char own own generics = alloc(
		sizeof(char own) * res->t.transform.from_len, NULL
	);
	res->t.transform.n_generics = 0;
	for (usz i = 0; i < block->stackspec.in_len; ++i) {
		const tok_t tok = block->stackspec.in_types[i];
		if (src[tok.pos.offset] == '\'') {
			for (int j = 0; j < res->t.transform.n_generics; ++j) {
				if (match(src, &tok, generics[i])) {
					goto generic_already_found;
				}
			}
			generics[res->t.transform.n_generics] = alloc(
				sizeof(char) * (tok.len+1), NULL
			);
			memmove(
				generics[res->t.transform.n_generics],
				src+tok.pos.offset, tok.len+1
			);
		generic_already_found:;
		}
	}

	for (usz i = 0; i < res->t.transform.from_len; ++i) {
		const tok_t tok = block->stackspec.in_types[i];
		if (src[tok.pos.offset] == '\'') {
			usz generic_num;
			for (generic_num = 0;
				!match(src, &tok, generics[generic_num]);
				++generic_num);

			typespec_init(&res->t.transform.from[i]);
			res->t.transform.from[i].ts.generic.idx = generic_num;
		} else {
			typespec_init(&res->t.transform.from[i]);
			res->t.transform.from[i].is_generic = false;
			type_copy(
				&res->t.transform.from[i].ts.type,
				type_defs_lookup_word(type_defs, &tok, src)
			);
		}
	}

	res->t.transform.to_len = 0;
	for (usz i = 0; i < block->stackspec.out_len; ++i) {
		const tok_t tok = block->stackspec.out_types[i];
		if (match(src, &tok, "'")) {
			res->t.transform.to_len += res->t.transform.from_len;
		} else {
			res->t.transform.to_len += 1;
		}
	}
	res->t.transform.to = alloc(
		sizeof(typespec_t) * res->t.transform.to_len, NULL
	);

	for (usz block_i = 0, res_i = 0; res_i < res->t.transform.to_len; ++block_i) {
		const tok_t tok = block->stackspec.out_types[block_i];
		if (match(src, &tok, "'")) {
			for (int j = 0; j < res->t.transform.from_len; ++j) {
				typespec_copy(
					&res->t.transform.to[res_i+j],
					&res->t.transform.from[j]
				);
			}
			res_i += res->t.transform.from_len;
		} else if (src[tok.pos.offset] == '\'') {
			usz generic_num;
			for (generic_num = 0;
				!match(src, &tok, generics[generic_num])
				&& generic_num < res->t.transform.n_generics;
				++generic_num);

			if (generic_num == res->t.transform.n_generics) {
				/* TODO: report & propogate type errors properly */
				fprints("TYPE ERROR: referencing a generic type in the output not present in the input", stderr, NULL);
				exit(1);
			}

			typespec_init(&res->t.transform.to[res_i]);
			res->t.transform.to[res_i].ts.generic.idx = generic_num;

			++res_i;
		} else {
			typespec_init(&res->t.transform.to[res_i]);
			res->t.transform.to[res_i].is_generic = false;
			type_copy(
				&res->t.transform.to[res_i].ts.type,
				type_defs_lookup_word(type_defs, &tok, src)
			);

			++res_i;
		}
	}

	return res;
}

type_t own
get_type(const item_t ref item, const char ref src,
	 const type_defs_t ref type_defs)
{
	switch (item->type) {
		case IT_NULL: {
			return NULL;
		break; }
		case IT_WORD: {
			type_t own res = alloc(sizeof(type_t), NULL);
			type_copy(res, type_defs_lookup_word(
				type_defs, &item->item.word, src
			));
		break; }
		case IT_ASSGN: {
			return gen_assgn_type();
		break; }
		case IT_BLOCK: {
			return copy_block_type(&item->item.block, src, type_defs);
		break; }
	}

	return NULL;
}
bool
match_typespec(const typespec_t ref ts1, const typespec_t ref ts2)
{
	if (ts1->is_generic != ts2->is_generic) return false;

	if (ts1->is_generic) {
		return ts1->ts.generic.idx == ts2->ts.generic.idx;
	} else {
		return match_type(&ts1->ts.type, &ts2->ts.type);
	}
}
bool
match_type(const type_t ref t1, const type_t ref t2)
{
	if (t1->type != t2->type) return false;

	switch (t1->type) {
		case TT_SIMPLE: {
			return t1->t.simple == t2->t.simple;
		break; }
		case TT_TRANSFORM: {
			if (t1->t.transform.n_generics != t2->t.transform.n_generics) {
				return false;
			}
			for (usz i = 0; i < t1->t.transform.from_len; ++i) {
				const typespec_t ref t1_from = &t1->t.transform.from[i];
				const typespec_t ref t2_from = &t2->t.transform.from[i];
				if (!match_typespec(t1_from, t2_from)) return false;
			}
			for (usz i = 0; i < t1->t.transform.to_len; ++i) {
				const typespec_t ref t1_to = &t1->t.transform.to[i];
				const typespec_t ref t2_to = &t2->t.transform.to[i];
				if (!match_typespec(t1_to, t2_to)) return false;
			}
			return true;
		break; }
	}
}
bool
check_transform(const transform_t ref trans, const type_stack_t ref type_stack)
{
	/* NOTE: assumes type stack's len is >= no. of input types */

	type_t ref intypes_start = type_stack->top - trans->from_len;
	type_t own generics = alloc(sizeof(type_t) * trans->n_generics, NULL);
	usz generics_allocated = 0;

	for (usz i = 0; i < trans->from_len; ++i) {
		type_t ref type = NULL;
		if (trans->from[i].is_generic) {
			usz generic_idx = trans->from[i].ts.generic.idx;
			type = &generics[generic_idx];
			if (generic_idx == generics_allocated) {
				type_copy(type, &intypes_start[i]);
				++generics_allocated;
				/* guaranteed match, no need checking */
				continue;
			} else if (generic_idx > generics_allocated) {
				/* should never happen! */
				fprints("Something went horribly wrong!\n", stderr, NULL);
				exit(1);
			}
		} else {
			type = &trans->from[i].ts.type;
		}

		if (!match_type(type, &intypes_start[i])) {
			for (usz i = 0; i < generics_allocated; ++i) {
				type_deinit(&generics[i]);
			}
			free(generics);
			return false;
		}
	}

	for (usz i = 0; i < generics_allocated; ++i) {
		type_deinit(&generics[i]);
	}
	free(generics);
	return true;
}
bool
check_type(const type_t ref type, const type_stack_t ref type_stack)
{
	usz min_stack_len;

	switch (type->type) {
		case TT_SIMPLE: {
			min_stack_len = 0;
		break; }
		case TT_TRANSFORM: {
			min_stack_len = type->t.transform.from_len;
		break; }
	}

	if (type_stack->top - type_stack->stack < min_stack_len) {
		return false;
	}

	switch (type->type) {
		case TT_SIMPLE: {
			/* A simple type always pushes a value to the stack,
			 * and does not depend on the current value of the stack,
			 * so the state of the stack is irrelevant
			 */
			return true;
		break; }
		case TT_TRANSFORM: {
			return check_transform(&type->t.transform, type_stack);
		break; }
	}
}
void
apply_type(const type_t ref type, type_stack_t ref type_stack)
{
	/* TODO: */
}
bool
check_and_apply_type(const type_t ref type, type_stack_t ref type_stack)
{
	/* TODO: */
}
bool
check_and_apply_block(const block_t ref block, const char ref src,
		      const type_defs_t ref type_defs,
		      type_stack_t ref type_stack)
{
}
bool
check_and_apply_item(const item_t ref item, const char ref src,
		     const type_defs_t ref type_defs,
		     type_stack_t ref type_stack)
{
}
