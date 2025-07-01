#ifndef LEX_H
#define LEX_H

#include "types.h"

bool match(const char ref src, const tok_t ref token, const char ref with);

tok_t parse_tok(const char ref src, pos_t ref pos);

#endif /* LEX_H */
