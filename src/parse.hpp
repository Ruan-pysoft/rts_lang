#ifndef PARSE_H
#define PARSE_H

#include <ostream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "lex.hpp"

enum AstType {
	AT_IDENT,
	AT_GENERIC,
	AT_LIST,
	AT_BLOCK,
};


class Ast {
	using Named = std::pair<bool, std::string>;
	using Listy = std::pair<bool, std::vector<Ast>>;
	std::variant<Named, Listy> content;
public:
	Ast(bool isGeneric, const std::string_view &name);
	Ast(bool isBlock, const std::vector<Ast> &list);

	AstType getType() const;
	const std::string &getIdent() const;
	const std::string &getGeneric() const;
	const std::vector<Ast> &getList() const;
	const std::vector<Ast> &getBlock() const;

	friend std::ostream &operator<<(std::ostream &os, const Ast &ast);
};

std::vector<Ast> parse(const std::string_view &src,
		       const std::vector<Token> &toks);

#endif /* PARSE_H */
