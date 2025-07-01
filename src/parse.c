#include "parse.h"

#include "ministd.h"
#include "ministd_fmt.h"
#include "ministd_memory.h"

#include "lex.h"

word_t parse_word(const char ref src, tok_t ref toks, usz len, usz ref idx,
		  perr_t ref perr_out);
stackspec_t parse_stackspec(const char ref src, tok_t ref toks, usz len,
			    usz ref idx, perr_t ref perr_out);
block_t parse_block(const char ref src, tok_t ref toks, usz len, usz ref idx,
		    perr_t ref perr_out);
assgn_t parse_assgn(const char ref src, tok_t ref toks, usz len, usz ref idx,
		    perr_t ref perr_out);

#define ERROR(res_type, tok, msg) do { \
		res_type ## _deinit(&res); \
		if (perr_out != NULL) { \
			perr_out->at_tok = tok; \
			perr_out->message = msg; \
		} else { \
			perr_t perr; \
			perr_init(&perr); \
			perr.at_tok = tok; \
			perr.message = msg; \
			perr_display(&perr, src, stderr, NULL); \
			exit(1); \
		} \
		return res; \
	} while (0)
#define TRY(res_type) do { \
		if (perr_out != NULL && perr_has_err(perr_out)) { \
			res_type ## _deinit(&res); \
			return res; \
		} \
	} while (0)

word_t
parse_word(const char ref src, tok_t ref toks, usz len, usz ref pos,
	   perr_t ref perr_out)
{
	/* NOTE: assumes *pos < len */

	/* A word consists of any single token */

	return toks[(*pos)++];
}
stackspec_t
parse_stackspec(const char ref src, tok_t ref toks, usz len, usz ref pos,
		perr_t ref perr_out)
{
	stackspec_t res;
	tok_t ref start;

	/* A stackspec consists of a list of types (each one token),
	 * followed by "->",
	 * followed by a second list of types (each one token)
	 * where "'" represents the input list,
	 * terminated by ":"
	 */

	stackspec_init(&res);

	start = toks + *pos;
	while (*pos < len && !match(src, &toks[*pos], "->")) {
		++*pos;
	}

	res.in_len = toks + *pos - start;
	res.in_types = alloc(sizeof(tok_t) * res.in_len, NULL);
	memmove(res.in_types, start, sizeof(tok_t) * res.in_len);

	if (*pos == len) {
		ERROR(stackspec, &toks[*pos-1],
		      "Hit eof while parsing block, expected \"->\"");
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
		ERROR(stackspec, &toks[*pos-1],
		      "Hit eof while parsing block, expected \":\"");
	}
	++*pos;

	return res;
}
block_t
parse_block(const char ref src, tok_t ref toks, usz len, usz ref pos,
	    perr_t ref perr_out)
{
	/* NOTE: assumes *pos < len */
	block_t res;

	/* A block is started by "[",
	 * followed by a stackspec,
	 * followed by a list of items to be executed,
	 * and terminated by "]"
	 */

	block_init(&res);

	res.stackspec = parse_stackspec(src, toks, len, pos, perr_out);
	TRY(block);

	usz cap = 16;
	res.items = alloc(sizeof(item_t) * cap, NULL);

	while (*pos < len && !match(src, &toks[*pos], "]")) {
		res.items[res.len++] = parse_item(src, toks, len, pos, perr_out);
		TRY(block);
		if (res.items[res.len-1].type == IT_NULL) {
			ERROR(block, &toks[*pos-1], "Something went wrong!");
		}
	}

	if (*pos == len) {
		ERROR(block, &toks[*pos-1],
		      "Hit eof while parsing block, expected \"]\"");
	}
	++*pos;

	return res;
}
assgn_t
parse_assgn(const char ref src, tok_t ref toks, usz len, usz ref pos,
	    perr_t ref perr_out)
{
	/* NOTE: assumes *pos < len */
	assgn_t res;

	/* An assignment is indicated by a ":=" token,
	 * which is immediately followed by the word being assigned to
	 */

	assgn_init(&res);

	if (!match(src, &toks[*pos], ":=")) {
		ERROR(assgn, &toks[*pos], "Expected \":=\"");
	}
	++*pos;

	if (*pos == len) {
		ERROR(assgn, &toks[*pos-1],
		      "Hit eof while parsing assignment, expected a word");
	}

	res.word = parse_word(src, toks, len, pos, perr_out);
	TRY(assgn);

	return res;
}
item_t
parse_item(const char ref src, tok_t ref toks, usz len, usz ref pos,
	   perr_t ref perr_out)
{
	item_t res;

	/* An item is a block, an assignment, or a word */

	item_init(&res);
	if (*pos == len) return res;

	if (match(src, &toks[*pos], "[")) {
		++*pos;
		res.type = IT_BLOCK;
		res.item.block = parse_block(src, toks, len, pos, perr_out);
		TRY(item);
	} else if (match(src, &toks[*pos], ":=")) {
		res.type = IT_ASSGN;
		res.item.assgn = parse_assgn(src, toks, len, pos, perr_out);
		TRY(item);
	} else {
		res.type = IT_WORD;
		res.item.word = parse_word(src, toks, len, pos, perr_out);
		TRY(item);
	}

	return res;
}
