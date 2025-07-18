#include "types.h"

#include "ministd_fmt.h"
#include "ministd_memory.h"
#include "ministd_string.h"

#include "lex.h"

/* lexing types */

void
pos_init(pos_t ref this)
{
	this->offset = 0;
	this->line = 1;
	this->line_pos = 1;
}
void
pos_deinit(pos_t ref this)
{ }
void
pos_display(const pos_t ref this, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	fprinti(this->line, file, &err);
	TRY_VOID(err);
	fprintc(',', file, &err);
	TRY_VOID(err);
	fprinti(this->line_pos, file, &err);
	TRY_VOID(err);
}

void
tok_init(tok_t ref this)
{
	pos_init(&this->pos);
	this->len = 0;
}
void
tok_deinit(tok_t ref this)
{
	pos_deinit(&this->pos);
}
void
tok_display(const tok_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;
	usz written = 0;

	while (written < this->len) {
		isz bytes_written = write(
			file, (ptr)(src + this->pos.offset + written),
			this->len - written, &err
		);
		TRY_VOID(err);
		if (bytes_written == 0) ERR_VOID(ERR_IO);

		written += bytes_written;
	}
}

/* parsing types */

void
perr_init(perr_t ref this)
{
	this->message = NULL;
	this->at_tok = NULL;
	this->assoc_err = ERR_OK;
}
void
perr_deinit(perr_t ref this)
{ perr_init(this); }
void
perr_display(const perr_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	fprints("Parse error", file, &err);
	TRY_VOID(err);
	if (this->at_tok != NULL) {
		fprints(" at token ", file, &err);
		TRY_VOID(err);
		tok_display(this->at_tok, src, file, &err);
		TRY_VOID(err);
		fprintc('@', file, &err);
		TRY_VOID(err);
		pos_display(&this->at_tok->pos, file, &err);
		TRY_VOID(err);
	}
	if (this->message != NULL) {
		fprints(":\n", file, &err);
		TRY_VOID(err);
	} else {
		fprintc('\n', file, &err);
		TRY_VOID(err);
	}
	if (this->message != NULL) {
		fprints(this->message, file, &err);
		TRY_VOID(err);
		fprintc('\n', file, &err);
		TRY_VOID(err);
	}
	if (this->assoc_err != ERR_OK) {
		fprints("With c stdlib error: ", file, &err);
		TRY_VOID(err);
		fprints(err_str(err), file, &err);
		TRY_VOID(err);
		fprintc('\n', file, &err);
		TRY_VOID(err);
	}
}
bool
perr_has_err(const perr_t ref this)
{
	return this->at_tok != NULL || this->message != NULL || this->assoc_err != ERR_OK;
}

void
word_init(word_t ref this)
{ tok_init(this); }
void
word_deinit(word_t ref this)
{ tok_deinit(this); }
void
word_display(const word_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{ tok_display(this, src, file, err_out); }

void
stackspec_init(stackspec_t ref this, pos_t pos)
{
	this->in_types = NULL;
	this->in_len = 0;
	this->out_types = NULL;
	this->out_len = 0;
	this->pos = pos;
}
void
stackspec_deinit(stackspec_t ref this)
{
	pos_deinit(&this->pos);
	if (this->in_types) {
		for (usz i = 0; i < this->in_len; ++i) {
			tok_deinit(&this->in_types[i]);
		}
		free(this->in_types);
	}
	this->in_len = 0;

	if (this->out_types) {
		for (usz i = 0; i < this->out_len; ++i) {
			tok_deinit(&this->out_types[i]);
		}
		free(this->out_types);
	}
	this->out_len = 0;
}
void
stackspec_display(const stackspec_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	for (usz i = 0; i < this->in_len; ++i) {
		tok_display(&this->in_types[i], src, file, &err);
		TRY_VOID(err);
		fprintc(' ', file, &err);
		TRY_VOID(err);
	}

	fprints("->", file, &err);
	TRY_VOID(err);

	for (usz i = 0; i < this->out_len; ++i) {
		fprintc(' ', file, &err);
		TRY_VOID(err);
		tok_display(&this->out_types[i], src, file, &err);
		TRY_VOID(err);
	}
}

void
block_init(block_t ref this, pos_t pos)
{
	stackspec_init(&this->stackspec, pos);
	this->items = NULL;
	this->len = 0;
	this->pos = pos;
}
void
block_deinit(block_t ref this)
{
	stackspec_deinit(&this->stackspec);
	if (this->items) {
		for (usz i = 0; i < this->len; ++i) {
			item_deinit(this->items + i);
		}
		free(this->items);
	}
	this->len = 0;
}
void
block_display(const block_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	fprints("[ ", stdout, &err);
	TRY_VOID(err);
	stackspec_display(&this->stackspec, src, file, &err);
	TRY_VOID(err);
	fprints(" : ", stdout, &err);

	for (usz i = 0; i < this->len; ++i) {
		item_display(&this->items[i], src, file, &err);
		TRY_VOID(err);
		fprintc(' ', file, &err);
		TRY_VOID(err);
	}

	fprintc(']', file, err_out);
}

void
assgn_init(assgn_t ref this)
{ word_init(&this->word); }
void
assgn_deinit(assgn_t ref this)
{ word_deinit(&this->word); }
void
assgn_display(const assgn_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	fprints(":= ", file, &err);
	TRY_VOID(err);
	word_display(&this->word, src, file, err_out);
}

void
item_init(item_t ref this)
{
	this->type = IT_NULL;
}
void
item_deinit(item_t ref this)
{
	switch (this->type) {
		case IT_WORD: {
			this->type = IT_NULL;
			word_deinit(&this->item.word);
		break; }
		case IT_BLOCK: {
			this->type = IT_NULL;
			block_deinit(&this->item.block);
		break; }
		case IT_ASSGN: {
			this->type = IT_NULL;
			assgn_deinit(&this->item.assgn);
		break; }

		case IT_NULL: break;
	}
}
void
item_display(const item_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{
	switch (this->type) {
		case IT_WORD: {
			word_display(&this->item.word, src, file, err_out);
		break; }
		case IT_BLOCK: {
			block_display(&this->item.block, src, file, err_out);
		break; }
		case IT_ASSGN: {
			assgn_display(&this->item.assgn, src, file, err_out);
		break; }
		case IT_NULL: {
			fprints("NULL", file, err_out);
		break; }
	}
}

/* typechecking types */

void
terr_init(terr_t ref this, pos_t pos)
{
	this->type = TET_OK;
	this->at.msg = NULL;
	this->pos = pos;
}
void
terr_deinit(terr_t ref this)
{
	this->type = TET_OK;
	this->at.msg = NULL;
	pos_deinit(&this->pos);
}
void
terr_display(const terr_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	switch (this->type) {
		case TET_OK: {
			fprints("No type error", file, err_out);
		break; }
		case TET_INPUT_MISMATCH: {
			fprints("Input type mismatch: ", file, &err);
			TRY_VOID(err);
			transform_display(this->at.trans, file, &err);
			TRY_VOID(err);
		break; }
		case TET_BLOCK_FAIL: {
			fprints("Block effect does not match output type: ", file, &err);
			TRY_VOID(err);
			block_display(this->at.block, src, file, &err);
			TRY_VOID(err);
		break; }
		case TET_OTHER: {
			fprints("Type error: ", file, &err);
			TRY_VOID(err);
			fprints(this->at.msg, file, &err);
			TRY_VOID(err);
		break; }
	}

	fprints(" @ ", file, &err);
	TRY_VOID(err);
	fprinti(this->pos.line, file, &err);
	TRY_VOID(err);
	fprintc(',', file, &err);
	TRY_VOID(err);
	fprinti(this->pos.line_pos, file, &err);
	TRY_VOID(err);
}
bool
terr_has_err(const terr_t ref this)
{
	return this != NULL && this->type != TET_OK;
}

void
simple_type_display(enum simple_type this, FILE ref file, err_t ref err_out)
{
	switch (this) {
		case ST_INT: {
			fprints("int", file, err_out);
		break; }
		case ST_BOOL: {
			fprints("bool", file, err_out);
		break; }
	}
}

void
transform_init(transform_t ref this, pos_t pos)
{
	this->n_generics = 0;
	this->from = NULL;
	this->from_len = 0;
	this->to = NULL;
	this->to_len = 0;
	this->pos = pos;
}
void
transform_deinit(transform_t ref this)
{
	this->n_generics = 0;
	if (this->from) {
		for (usz i = 0; i < this->from_len; ++i) {
			type_deinit(&this->from[i]);
		}
		free(this->from);
		this->from = NULL;
	}
	this->from_len = 0;
	if (this->to) {
		for (usz i = 0; i < this->to_len; ++i) {
			type_deinit(&this->to[i]);
		}
		free(this->to);
		this->to = NULL;
	}
	this->to_len = 0;
}
void
transform_display(const transform_t ref this, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	fprints("[ ", file, &err);
	TRY_VOID(err);

	for (usz i = 0; i < this->from_len; ++i) {
		type_display(&this->from[i], file, &err);
		TRY_VOID(err);
		fprintc(' ', file, &err);
		TRY_VOID(err);
	}

	fprints("->", file, &err);
	TRY_VOID(err);

	for (usz i = 0; i < this->to_len; ++i) {
		fprintc(' ', file, &err);
		TRY_VOID(err);
		type_display(&this->to[i], file, &err);
		TRY_VOID(err);
	}

	fprints(" ]", file, &err);
	TRY_VOID(err);
}
void
transform_copy(transform_t ref to, const transform_t ref from)
{
	to->n_generics = from->n_generics;
	to->from_len = from->from_len;
	to->to_len = from->to_len;
	to->pos = from->pos;

	to->from = alloc(sizeof(type_t) * to->from_len, NULL);
	for (usz i = 0; i < to->from_len; ++i) {
		type_copy(&to->from[i], &from->from[i]);
	}

	to->to = alloc(sizeof(type_t) * to->to_len, NULL);
	for (usz i = 0; i < to->to_len; ++i) {
		type_copy(&to->to[i], &from->to[i]);
	}
}

void
type_init(type_t ref this, pos_t pos)
{
	this->type = TT_SIMPLE;
	this->t.simple = ST_INT;
	this->pos = pos;
}
void
type_deinit(type_t ref this)
{
	switch (this->type) {
		case TT_SIMPLE: {
		break; }
		case TT_GENERIC: {
		break; }
		case TT_TRANSFORM: {
			transform_deinit(&this->t.trans);
		break; }
	}
	this->type = TT_SIMPLE;
	this->t.simple = ST_INT;
	pos_deinit(&this->pos);
}
void
type_display(const type_t ref this, FILE ref file, err_t ref err_out)
{
	switch (this->type) {
		case TT_SIMPLE: {
			simple_type_display(this->t.simple, file, err_out);
		break; }
		case TT_GENERIC: {
			err_t err = ERR_OK;

			fprintc('\'', file, &err);
			TRY_VOID(err);
			fprintuz(this->t.gen, file, err_out);
		break; }
		case TT_TRANSFORM: {
			transform_display(&this->t.trans, file, err_out);
		break; }
	}
}
void
type_copy(type_t ref to, const type_t ref from)
{
	to->type = from->type;
	to->pos = from->pos;
	switch (from->type) {
		case TT_SIMPLE: {
			to->t.simple = from->t.simple;
		break; }
		case TT_GENERIC: {
			to->t.gen = from->t.gen;
		break; }
		case TT_TRANSFORM: {
			transform_copy(&to->t.trans, &from->t.trans);
		break; }
	}
}

void
type_stack_init(type_stack_t ref this)
{
	this->cap = 256;
	this->stack = alloc(sizeof(type_t) * this->cap, NULL);
	this->len = 0;
}
void
type_stack_deinit(type_stack_t ref this)
{
	if (this->stack) {
		for (usz i = 0; i < this->len; ++i) {
			type_deinit(&this->stack[i]);
		}
		free(this->stack);
	}
	this->stack = NULL;
	this->len = 0;
	this->cap = 0;
}
void
type_stack_display(const type_stack_t ref this, const char ref src,
		   FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	for (usz i = 0; i < this->len; ++i) {
		if (i != 0) {
			fprintc(' ', file, &err);
			TRY_VOID(err);
		}
		type_display(&this->stack[i], file, &err);
		TRY_VOID(err);
	}
}
const type_t ref
type_stack_peek(const type_stack_t ref this)
{
	if (this->len == 0) return NULL;
	return &this->stack[this->len - 1];
}
void
type_stack_pop(type_stack_t ref this)
{
	if (this->len == 0) return;
	--this->len;
	type_deinit(&this->stack[this->len]);
}
void
type_stack_push(type_stack_t ref this, const type_t ref type)
{
	if (this->len == this->cap) {
		this->stack = realloc(
			this->stack, sizeof(type_t) * this->cap * 2, NULL
		);
		this->cap *= 2;
	}
	type_copy(&this->stack[this->len++], type);
}

void
generic_map_init(generic_map_t ref this, usz n_generics)
{
	this->cap = n_generics;
	this->types = alloc(sizeof(type_t) * n_generics, NULL);
	this->len = 0;
}
void
generic_map_add(generic_map_t ref this, const type_t ref type)
{
	if (this->len == this->cap) return;

	type_copy(&this->types[this->len++], type);
}
void
generic_map_deinit(generic_map_t ref this)
{
	if (this->types) {
		for (usz i = 0; i < this->len; ++i) {
			type_deinit(&this->types[i]);
		}
		free(this->types);
		this->types = NULL;
	}
	this->len = 0;
	this->cap = 0;
}
void
generic_map_display(const generic_map_t ref this, FILE ref file,
		    err_t ref err_out)
{
	err_t err = ERR_OK;

	fprintc('{', file, &err);
	TRY_VOID(err);
	for (usz i = 0; i < this->len; ++i) {
		if (i != 0) {
			fprints(", ", file, &err);
			TRY_VOID(err);
		}
		fprintc('\'', file, &err);
		TRY_VOID(err);
		fprintuz(i, file, &err);
		TRY_VOID(err);
		fprints(": ", file, &err);
		TRY_VOID(err);
		type_display(&this->types[i], file, &err);
		TRY_VOID(err);
	}
	fprintc('}', file, &err);
	TRY_VOID(err);
}

void
type_defs_init(type_defs_t ref this)
{
	this->cap = 256;
	this->names = alloc(sizeof(char own) * this->cap, NULL);
	this->types = alloc(sizeof(type_t) * this->cap, NULL);
	this->len = 0;
}
void
type_defs_deinit(type_defs_t ref this)
{
	if (this->names) {
		for (usz i = 0; i < this->len; ++i) {
			free(this->names[i]);
		}
		free(this->names);
		this->names = NULL;
	}
	if (this->types) {
		for (usz i = 0; i < this->len; ++i) {
			type_deinit(&this->types[i]);
		}
		free(this->types);
		this->types = NULL;
	}
	this->cap = 0;
	this->len = 0;
}
void
type_defs_display(const type_defs_t ref this, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	fprintc('{', file, &err);
	TRY_VOID(err);
	for (usz i = 0; i < this->len; ++i) {
		if (i != 0) {
			fprints(", ", file, &err);
			TRY_VOID(err);
		}
		fprints(this->names[i], file, &err);
		TRY_VOID(err);
		fprints(": ", file, &err);
		TRY_VOID(err);
		type_display(&this->types[i], file, &err);
		TRY_VOID(err);
	}
	fprintc('}', file, &err);
	TRY_VOID(err);
}
bool
strcmp(const char ref s1, const char ref s2)
{
	for (; *s1 != 0 && *s2 != 0 && s1 == s2; ++s1, ++s2);
	return *s1 == 0;
}
const type_t ref
type_defs_lookup(const type_defs_t ref this, const char ref name)
{
	for (usz i = 0; i < this->len; ++i) {
		if (strcmp(this->names[i], name)) {
			return &this->types[i];
		}
	}

	return NULL;
}
const type_t ref
type_defs_lookup_word(const type_defs_t ref this, const word_t ref word,
		      const char ref src)
{
	for (usz i = 0; i < this->len; ++i) {
		if (match(src, word, this->names[i])) {
			return &this->types[i];
		}
	}

	return NULL;
}
void
type_defs_remove_at(type_defs_t ref this, usz ix) {
	free(this->names[ix]);
	type_deinit(&this->types[ix]);
	--this->len;

	for (usz i = ix; i < this->len; ++i) {
		this->names[i] = this->names[i+1];
		this->types[i] = this->types[i+1];
	}
}
void
type_defs_remove(type_defs_t ref this, const char ref name)
{
	for (usz i = 0; i < this->len; ++i) {
		if (strcmp(this->names[i], name)) {
			type_defs_remove_at(this, i);
			return;
		}
	}
}
void
type_defs_remove_word(type_defs_t ref this, const word_t ref word,
		      const char ref src)
{
	for (usz i = 0; i < this->len; ++i) {
		if (match(src, word, this->names[i])) {
			type_defs_remove_at(this, i);
			return;
		}
	}
}
void
type_defs_add(type_defs_t ref this, const char ref name, const type_t ref type)
{
	for (usz i = 0; i < this->len; ++i) {
		if (strcmp(this->names[i], name)) {
			type_deinit(&this->types[i]);
			type_copy(&this->types[i], type);
			return;
		}
	}

	if (this->len == this->cap) {
		this->cap *= 2;
		this->names = realloc(this->names, sizeof(char own) * this->cap, NULL);
		this->types = realloc(this->types, sizeof(type_t) * this->cap, NULL);
	}
	this->names[this->len] = alloc(sizeof(char) * (strlen(name)+1), NULL);
	memmove(this->names[this->len], name, strlen(name)+1);
	type_copy(&this->types[this->len], type);
	++this->len;
}

void
type_defs_add_word(type_defs_t ref this, const word_t ref word,
		   const char ref src, const type_t ref type)
{
	for (usz i = 0; i < this->len; ++i) {
		if (match(src, word, this->names[i])) {
			type_deinit(&this->types[i]);
			type_copy(&this->types[i], type);
			return;
		}
	}

	if (this->len == this->cap) {
		this->cap *= 2;
		this->names = realloc(this->names, sizeof(char own) * this->cap, NULL);
		this->types = realloc(this->types, sizeof(type_t) * this->cap, NULL);
	}
	this->names[this->len] = alloc(sizeof(char) * (word->len+1), NULL);
	memmove(this->names[this->len], &src[word->pos.offset], word->len+1);
	type_copy(&this->types[this->len], type);
	++this->len;
}
