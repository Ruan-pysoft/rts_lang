#ifndef TYPES_H
#define TYPES_H

#include "ministd_error.h"
#include "ministd_io.h"
#include "ministd_types.h"

/* lexing types */

typedef struct pos_t {
	usz offset;
	usz line;
	usz line_pos;
} pos_t;
void pos_init(pos_t ref this);
void pos_deinit(pos_t ref this);
void pos_display(const pos_t ref this, FILE ref file, err_t ref err_out);

typedef struct tok_t {
	pos_t pos;
	usz len;
} tok_t;
void tok_init(tok_t ref this);
void tok_deinit(tok_t ref this);
void tok_display(const tok_t ref this, const char ref src, FILE ref file, err_t ref err_out);

/* parsing types */

typedef struct perr_t {
	const char ref message;
	const tok_t ref at_tok;
	err_t assoc_err;
} perr_t;
void perr_init(perr_t ref this);
void perr_deinit(perr_t ref this);
void perr_display(const perr_t ref this, const char ref src, FILE ref file, err_t ref err_out);
bool perr_has_err(const perr_t ref this);

struct item_t;

typedef tok_t word_t;
void word_init(word_t ref this);
void word_deinit(word_t ref this);
void word_display(const word_t ref this, const char ref src, FILE ref file, err_t ref err_out);

typedef struct stackspec_t {
	tok_t own in_types;
	usz in_len;
	tok_t own out_types;
	usz out_len;
} stackspec_t;
void stackspec_init(stackspec_t ref this);
void stackspec_deinit(stackspec_t ref this);
void stackspec_display(const stackspec_t ref this, const char ref src, FILE ref file, err_t ref err_out);

typedef struct block_t {
	stackspec_t stackspec;
	struct item_t own items;
	usz len;
} block_t;
void block_init(block_t ref this);
void block_deinit(block_t ref this);
void block_display(const block_t ref this, const char ref src, FILE ref file, err_t ref err_out);

typedef struct assgn_t {
	word_t word;
} assgn_t;
void assgn_init(assgn_t ref this);
void assgn_deinit(assgn_t ref this);
void assgn_display(const assgn_t ref this, const char ref src, FILE ref file, err_t ref err_out);

enum item_type {
	IT_WORD,
	IT_BLOCK,
	IT_ASSGN,

	IT_NULL
};

typedef struct item_t {
	enum item_type type;
	union item_union {
		word_t word;
		block_t block;
		assgn_t assgn;
	} item;
} item_t;
void item_init(item_t ref this);
void item_deinit(item_t ref this);
void item_display(const item_t ref this, const char ref src, FILE ref file,
		  err_t ref err_out);

/* typechecking types */

enum simple_type {
	ST_INT,
	ST_BOOL,
};
void simple_type_display(enum simple_type this, FILE ref file, err_t ref err_out);

struct typespec_t;
typedef struct transform_t {
	usz n_generics;
	struct typespec_t own from;
	usz from_len;
	struct typespec_t own to;
	usz to_len;
} transform_t;
void transform_init(transform_t ref this);
void transform_deinit(transform_t ref this);
void transform_display(const transform_t ref this, FILE ref file,
		       err_t ref err_out);
void transform_copy(transform_t ref to, const transform_t ref from);

enum type_type {
	TT_SIMPLE,
	TT_TRANSFORM,
};

typedef struct type_t {
	enum type_type type;
	union type_union {
		enum simple_type simple;
		transform_t transform;
	} t;
} type_t;
void type_init(type_t ref this);
void type_deinit(type_t ref this);
void type_display(const type_t ref this, FILE ref file, err_t ref err_out);
void type_copy(type_t ref to, const type_t ref from);

typedef struct generic_t {
	unsigned idx;
} generic_t;
void generic_init(generic_t ref this);
void generic_deinit(generic_t ref this);
void generic_display(const generic_t ref this, FILE ref file,
		     err_t ref err_out);

typedef struct typespec_t {
	bool is_generic;
	union typespec_union {
		type_t type;
		generic_t generic;
	} ts;
} typespec_t;
void typespec_init(typespec_t ref this);
void typespec_deinit(typespec_t ref this);
void typespec_display(const typespec_t ref this, FILE ref file,
		      err_t ref err_out);
void typespec_copy(typespec_t ref to, const typespec_t ref from);

typedef struct type_stack_t {
	type_t own stack;
	type_t ref top;
	usz cap;
} type_stack_t;
void type_stack_init(type_stack_t ref this);
void type_stack_deinit(type_stack_t ref this);
void type_stack_display(const type_stack_t ref this, const char ref src,
			FILE ref file, err_t ref err_out);
const type_t ref type_stack_peek(const type_stack_t ref this);
void type_stack_pop(type_stack_t ref this);
void type_stack_push(type_stack_t ref this, const type_t ref type);

typedef struct generic_map_t {
	/* generics are labeled sequentially starting with 0,
	 * no need for an array of generics */
	type_t own types;
	usz len;
} generic_map_t;
void generic_map_init(generic_map_t ref this);
void generic_map_deinit(generic_map_t ref this);
void generic_map_display(const generic_map_t ref this, FILE ref file,
			 err_t ref err_out);

typedef struct type_defs_t {
	char own own names;
	type_t own types;
	usz cap;
	usz len;
} type_defs_t;
void type_defs_init(type_defs_t ref this);
void type_defs_deinit(type_defs_t ref this);
void type_defs_display(const type_defs_t ref this, FILE ref file,
		       err_t ref err_out);
const type_t ref type_defs_lookup(const type_defs_t ref this,
				  const char ref name);
const type_t ref type_defs_lookup_word(const type_defs_t ref this,
				       const word_t ref word,
				       const char ref src);
void type_defs_remove(type_defs_t ref this, const char ref name);
void type_defs_remove_word(type_defs_t ref this, const word_t ref word,
			   const char ref src);
void type_defs_add(type_defs_t ref this, const char ref name,
		   const type_t ref type);
void type_defs_add_word(type_defs_t ref this, const word_t ref word,
			const char ref src, const type_t ref type);

#endif /* TYPES_H */
