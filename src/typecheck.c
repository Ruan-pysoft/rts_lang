#include "typecheck.h"

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
	res->t.transform.from_len = 1;
	res->t.transform.from = alloc(sizeof(typespec_t), NULL);
	typespec_init(res->t.transform.from);

	return res;
}
type_t own
copy_block_type(const block_t ref block, const char ref src)
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
	usz generics_len = 0;
	for (int i = 0; i < block->stackspec.in_len; ++i) {
		tok_t tok = block->stackspec.in_types[i];
		if (src[tok.pos.offset] == '\'') {
			for (int j = 0; j < generics_len; ++j) {
				if (match(src, &tok, generics[i])) {
					goto generic_already_found;
				}
			}
			generics[generics_len] = alloc(
				sizeof(char) * (tok.len+1), NULL
			);
			memmove(
				generics[generics_len], src+tok.pos.offset,
				tok.len+1
			);
		generic_already_found:;
		}
	}

	/* TODO: in types */

	/* TODO: out types */

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
			return copy_block_type(&item->item.block, src);
		break; }
	}

	return NULL;
}
bool
check_type(const type_t ref type, const type_stack_t ref type_stack)
{
	usz min_stack_len;

	switch (type->type) {
		case TT_SIMPLE: {
			min_stack_len = 1;
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
			return type_stack->top[-1].type == TT_SIMPLE
				&& type_stack->top[-1].t.simple == type->t.simple;
		break; }
		case TT_TRANSFORM: {
			/* TODO: */
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
