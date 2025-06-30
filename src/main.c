#include "ministd.h"
#include "ministd_fmt.h"
#include "ministd_io.h"
#include "ministd_memory.h"
#include "ministd_string.h"

const char ref src =
	"[ 'a 'b 'c -> 'c 'a 'b : rot rot ] := unrot\n"
	"[ int -> ' :\n"
	"  0 1\n"
	"  [ int int int -> ' bool : rot 0 > swp drop ]\n"
	"  [ int int int -> ' : unrot dup rot + ]\n"
	"  while\n"
	"] := fib\n"
	"10 fib\n"
;

typedef struct pos_t {
	usz offset;
	usz line;
	usz line_pos;
} pos_t;

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

typedef struct tok_t {
	pos_t pos;
	usz len;
} tok_t;

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

bool
match(const char ref src, tok_t ref token, const char ref with)
{
	const usz len = strnlen(with, token->len+1);

	if (len != token->len) return false;

	for (usz i = 0; i < len; ++i) {
		if (src[token->pos.offset + i] != with[i]) return false;
	}

	return true;
}

void
tok_display(const char ref src, tok_t ref token, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;
	usz written = 0;

	while (written < token->len) {
		isz bytes_written = write(
			file, (ptr)(src + token->pos.offset + written),
			token->len - written, &err
		);
		TRY_VOID(err);
		if (bytes_written == 0) ERR_VOID(ERR_IO);

		written += bytes_written;
	}
}

tok_t
parse_tok(const char ref src, pos_t ref pos)
{
	tok_t res;

	tok_init(&res);

	while (src[pos->offset] != 0 && src[pos->offset] <= ' ') {
		if (src[pos->offset] == '\n') {
			pos->line++;
			pos->line_pos = 0;
		}
		pos->offset++;
		pos->line_pos++;
	}

	res.pos = *pos;

	while (src[res.pos.offset + res.len] != 0 && src[res.pos.offset + res.len] > ' ') {
		res.len++;
		pos->offset++;
		pos->line_pos++;
	}

	return res;
}

struct item_t;
void item_init(struct item_t ref this);
void item_deinit(struct item_t ref this);

typedef tok_t word_t;

void
word_init(word_t ref this)
{ tok_init(this); }
void
word_deinit(word_t ref this)
{ tok_deinit(this); }

typedef struct stackspec_t {
	tok_t own in_types;
	usz in_len;
	tok_t own out_types;
	usz out_len;
} stackspec_t;

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

typedef struct block_t {
	stackspec_t stackspec;
	struct item_t own items;
	usz len;
} block_t;

void
block_init(block_t ref this)
{
	stackspec_init(&this->stackspec);
	this->items = NULL;
	this->len = 0;
}
void block_deinit(block_t ref this);

typedef struct assgn_t {
	word_t word;
} assgn_t;

void
assgn_init(assgn_t ref this)
{ word_init(&this->word); }
void
assgn_deinit(assgn_t ref this)
{ word_deinit(&this->word); }

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

item_t parse_item(const char ref src, tok_t ref toks, usz len, usz ref pos);
void item_display(const char ref src, item_t ref item, FILE ref file, err_t ref err_out);

word_t
parse_word(const char ref src, tok_t ref toks, usz len, usz ref pos)
{
	/* NOTE: assumes *pos < len */
	return toks[(*pos)++];
}
void
word_display(const char ref src, word_t ref word, FILE ref file, err_t ref err_out)
{ tok_display(src, word, file, err_out); }

stackspec_t
parse_stackspec(const char ref src, tok_t ref toks, usz len, usz ref pos)
{
	stackspec_t res;
	tok_t ref start;

	stackspec_init(&res);

	start = toks + *pos;
	while (*pos < len && !match(src, &toks[*pos], "->")) {
		++*pos;
	}

	res.in_len = toks + *pos - start;
	res.in_types = alloc(sizeof(tok_t) * res.in_len, NULL);
	memmove(res.in_types, start, sizeof(tok_t) * res.in_len);

	if (*pos == len) {
		fputs("Error at token ", stderr, NULL);
		tok_display(src, &toks[*pos-1], stderr, NULL);
		printc('@', NULL);
		printi(toks[*pos-1].pos.line, NULL);
		printc(',', NULL);
		printi(toks[*pos-1].pos.line_pos, NULL);
		fputs(": Hit eof while parsing block, expected \"->\"", stderr, NULL);
		exit(1);
	}
	++*pos;

	tok_t ref out_start = toks + *pos;
	res.out_len = 0;
	while (*pos < len && !match(src, &toks[*pos], ":")) {
		if (match(src, &toks[*pos], "'")) {
			res.out_len += res.in_len;
		} else {
			++res.out_len;
		}
		++*pos;
	}

	res.out_types = alloc(sizeof(tok_t) * res.out_len, NULL);
	for (usz i = 0; i < res.out_len; ++i, ++out_start) {
		if (match(src, out_start, "'")) {
			memmove(&res.out_types[i], start, sizeof(tok_t) * res.in_len);
			i += res.in_len;
			--i;
		} else {
			res.out_types[i] = *out_start;
		}
	}

	if (*pos == len) {
		fputs("Error at token ", stderr, NULL);
		tok_display(src, &toks[*pos-1], stderr, NULL);
		printc('@', NULL);
		printi(toks[*pos-1].pos.line, NULL);
		printc(',', NULL);
		printi(toks[*pos-1].pos.line_pos, NULL);
		fputs(": Hit eof while parsing block, expected \":\"", stderr, NULL);
		exit(1);
	}
	++*pos;

	return res;
}
void
stackspec_display(const char ref src, stackspec_t ref stackspec, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	for (usz i = 0; i < stackspec->in_len; ++i) {
		tok_display(src, &stackspec->in_types[i], file, &err);
		TRY_VOID(err);
		fputc(' ', file, &err);
		TRY_VOID(err);
	}

	fputs("->", file, &err);
	TRY_VOID(err);

	for (usz i = 0; i < stackspec->out_len; ++i) {
		fputc(' ', file, &err);
		TRY_VOID(err);
		tok_display(src, &stackspec->out_types[i], file, &err);
		TRY_VOID(err);
	}
}

block_t
parse_block(const char ref src, tok_t ref toks, usz len, usz ref pos)
{
	/* NOTE: assumes *pos < len */
	block_t res;

	block_init(&res);

	res.stackspec = parse_stackspec(src, toks, len, pos);

	usz cap = 16;
	res.items = alloc(sizeof(item_t) * cap, NULL);

	while (*pos < len && !match(src, &toks[*pos], "]")) {
		res.items[res.len++] = parse_item(src, toks, len, pos);
		if (res.items[res.len-1].type == IT_NULL) {
			fputs("Something went wrong! ", stderr, NULL);
			tok_display(src, &toks[*pos-1], stderr, NULL);
			printc('@', NULL);
			printi(toks[*pos-1].pos.line, NULL);
			printc(',', NULL);
			printi(toks[*pos-1].pos.line_pos, NULL);
			exit(1);
		}
	}

	if (*pos == len) {
		fputs("Error at token ", stderr, NULL);
		tok_display(src, &toks[*pos-1], stderr, NULL);
		printc('@', NULL);
		printi(toks[*pos-1].pos.line, NULL);
		printc(',', NULL);
		printi(toks[*pos-1].pos.line_pos, NULL);
		fputs(": Hit eof while parsing block, expected \"]\"", stderr, NULL);
		exit(1);
	}
	++*pos;

	return res;
}
void
block_display(const char ref src, block_t ref word, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	fputs("[ ", stdout, &err);
	TRY_VOID(err);
	stackspec_display(src, &word->stackspec, file, &err);
	TRY_VOID(err);
	fputs(" : ", stdout, &err);

	for (usz i = 0; i < word->len; ++i) {
		item_display(src, &word->items[i], file, &err);
		TRY_VOID(err);
		fputc(' ', file, &err);
		TRY_VOID(err);
	}

	fputc(']', file, err_out);
}

assgn_t
parse_assgn(const char ref src, tok_t ref toks, usz len, usz ref pos)
{
	/* NOTE: assumes *pos < len */
	assgn_t res;

	assgn_init(&res);

	if (!match(src, &toks[*pos], ":=")) {
		fputs("Error at token ", stderr, NULL);
		tok_display(src, &toks[*pos], stderr, NULL);
		printc('@', NULL);
		printi(toks[*pos].pos.line, NULL);
		printc(',', NULL);
		printi(toks[*pos].pos.line_pos, NULL);
		fputs(": Expected \":=\"", stderr, NULL);
		exit(1);
	}
	++*pos;

	if (*pos == len) {
		fputs("Error at token ", stderr, NULL);
		tok_display(src, &toks[*pos-1], stderr, NULL);
		printc('@', NULL);
		printi(toks[*pos-1].pos.line, NULL);
		printc(',', NULL);
		printi(toks[*pos-1].pos.line_pos, NULL);
		fputs(": Hit eof while parsing assignment, expected a word", stderr, NULL);
		exit(1);
	}

	res.word = parse_word(src, toks, len, pos);

	return res;
}
void
assgn_display(const char ref src, assgn_t ref assgn, FILE ref file, err_t ref err_out)
{
	err_t err = ERR_OK;

	fputs(":= ", file, &err);
	TRY_VOID(err);
	word_display(src, &assgn->word, file, err_out);
}

item_t
parse_item(const char ref src, tok_t ref toks, usz len, usz ref pos)
{
	item_t res;

	item_init(&res);
	if (*pos == len) return res;

	if (match(src, &toks[*pos], "[")) {
		++*pos;
		res.type = IT_BLOCK;
		res.item.block = parse_block(src, toks, len, pos);
	} else if (match(src, &toks[*pos], ":=")) {
		res.type = IT_ASSGN;
		res.item.assgn = parse_assgn(src, toks, len, pos);
	} else {
		res.type = IT_WORD;
		res.item.word = parse_word(src, toks, len, pos);
	}

	return res;
}
void
item_display(const char ref src, item_t ref item, FILE ref file, err_t ref err_out)
{
	switch (item->type) {
		case IT_WORD: {
			word_display(src, &item->item.word, file, err_out);
		break; }
		case IT_BLOCK: {
			block_display(src, &item->item.block, file, err_out);
		break; }
		case IT_ASSGN: {
			assgn_display(src, &item->item.assgn, file, err_out);
		break; }
		case IT_NULL: {
			fputs("NULL", file, err_out);
		break; }
	}
}

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

		tok_display(src, &token, stdout, NULL);
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
		item = parse_item(src, toks, toks_len, &toks_pos);

		item_display(src, &item, stdout, NULL);
		putc('\n', NULL);
	}

	return 0;
}
