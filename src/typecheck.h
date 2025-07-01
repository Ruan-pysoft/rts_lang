#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "types.h"

type_t own get_type(const item_t ref item, const char ref src,
		    const type_defs_t ref type_defs);
bool check_type(const type_t ref type, const type_stack_t ref type_stack);
void apply_type(const type_t ref type, type_stack_t ref type_stack);
bool check_and_apply_type(const type_t ref type, type_stack_t ref type_stack);
bool check_and_apply_block(const block_t ref block, const char ref src,
			   const type_defs_t ref type_defs,
			   type_stack_t ref type_stack);
bool check_and_apply_item(const item_t ref item, const char ref src,
			  const type_defs_t ref type_defs,
			  type_stack_t ref type_stack);

#endif /* TYPECHECK_H */
