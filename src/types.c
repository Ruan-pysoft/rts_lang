#include "types.h"

#include "ministd_memory.h"

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
tok_display(tok_t ref this, const char ref src, FILE ref file, err_t ref err_out)
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

void
word_init(word_t ref this)
{ tok_init(this); }
void
word_deinit(word_t ref this)
{ tok_deinit(this); }
void
word_display(word_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{ tok_display(this, src, file, err_out); }

void
stackspec_init(stackspec_t ref this)
{
	this->in_types = NULL;
	this->in_len = 0;
	this->out_types = NULL;
	this->out_len = 0;
}
void
stackspec_deinit(stackspec_t ref this)
{
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
stackspec_display(stackspec_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	for (usz i = 0; i < this->in_len; ++i) {
		tok_display(&this->in_types[i], src, file, &err);
		TRY_VOID(err);
		fputc(' ', file, &err);
		TRY_VOID(err);
	}

	fputs("->", file, &err);
	TRY_VOID(err);

	for (usz i = 0; i < this->out_len; ++i) {
		fputc(' ', file, &err);
		TRY_VOID(err);
		tok_display(&this->out_types[i], src, file, &err);
		TRY_VOID(err);
	}
}

void
block_init(block_t ref this)
{
	stackspec_init(&this->stackspec);
	this->items = NULL;
	this->len = 0;
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
block_display(block_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	fputs("[ ", stdout, &err);
	TRY_VOID(err);
	stackspec_display(&this->stackspec, src, file, &err);
	TRY_VOID(err);
	fputs(" : ", stdout, &err);

	for (usz i = 0; i < this->len; ++i) {
		item_display(&this->items[i], src, file, &err);
		TRY_VOID(err);
		fputc(' ', file, &err);
		TRY_VOID(err);
	}

	fputc(']', file, err_out);
}

void
assgn_init(assgn_t ref this)
{ word_init(&this->word); }
void
assgn_deinit(assgn_t ref this)
{ word_deinit(&this->word); }
void
assgn_display(assgn_t ref this, const char ref src, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	fputs(":= ", file, &err);
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
item_display(item_t ref this, const char ref src, FILE ref file, err_t ref err_out)
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
			fputs("NULL", file, err_out);
		break; }
	}
}
