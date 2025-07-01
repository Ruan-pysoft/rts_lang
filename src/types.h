#ifndef TYPES_H
#define TYPES_H

#include "ministd_error.h"
#include "ministd_io.h"
#include "ministd_types.h"

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
void item_display(const item_t ref this, const char ref src, FILE ref file, err_t ref err_out);

#endif /* TYPES_H */
