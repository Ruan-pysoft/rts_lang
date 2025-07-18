#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "types.h"

void exec_type(type_stack_t ref type_stack, const type_t ref type,
	       terr_t ref err_out);
bool resolve_type(type_t ref type_out, const item_t ref item,
		  const char ref src, const type_defs_t ref type_defs);
void type_stack_start(type_stack_t ref type_stack_out,
		      const transform_t ref transform);
bool type_stack_check(const type_stack_t ref type_stack,
		      const transform_t ref transform);
void check_block(const block_t ref block, const char ref src,
		 const type_defs_t ref type_defs, terr_t ref err_out);
void apply_item(type_stack_t ref type_stack, const item_t ref item,
		const char ref src, const type_defs_t ref type_defs,
		terr_t ref err_out);
void apply_item_toplevel(type_stack_t ref type_stack, const item_t ref item,
			 const char ref src, type_defs_t ref type_defs,
			 terr_t ref err_out);

#endif /* TYPECHECK_H */
