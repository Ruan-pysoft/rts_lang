#ifndef LEX_H
#define LEX_H

#include <cstddef>
#include <string>
#include <vector>

struct Token {
	size_t pos;
	size_t len;

	Token(size_t pos, size_t len) : pos(pos), len(len) {}

	std::string_view get_tok(const std::string_view &str) const;
};

std::vector<Token> tokenize(const std::string_view &str);

#endif /* LEX_H */
