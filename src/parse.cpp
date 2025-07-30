#include "parse.hpp"

#include <cstring>
#include <exception>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

Ast::Ast(bool isGeneric, const std::string_view &name)
	: content(std::make_pair(isGeneric, std::string(name))) {}
Ast::Ast(bool isBlock, const std::vector<Ast> &list)
	: content(std::make_pair(isBlock, std::move(list))) {}

AstType Ast::getType() const {
	if (std::holds_alternative<Named>(content)) {
		auto &named = std::get<Named>(content);
		return named.first ? AT_GENERIC : AT_IDENT;
	} else {
		auto &listy = std::get<Listy>(content);
		return listy.first ? AT_BLOCK : AT_LIST;
	}
}
const std::string &Ast::getIdent() const {
	auto &named = std::get<Named>(content);
	if (named.first) throw std::bad_variant_access();
	return named.second;
}
const std::string &Ast::getGeneric() const {
	auto &named = std::get<Named>(content);
	if (!named.first) throw std::bad_variant_access();
	return named.second;
}
const std::vector<Ast> &Ast::getList() const {
	auto &listy = std::get<Listy>(content);
	if (listy.first) throw std::bad_variant_access();
	return listy.second;
}
const std::vector<Ast> &Ast::getBlock() const {
	auto &listy = std::get<Listy>(content);
	if (!listy.first) throw std::bad_variant_access();
	return listy.second;
}

std::ostream &operator<<(std::ostream &os, const Ast &ast) {
	if (std::holds_alternative<Ast::Named>(ast.content)) {
		auto &named = std::get<Ast::Named>(ast.content);
		os << named.second;
	} else {
		bool first = true;
		auto &listy = std::get<Ast::Listy>(ast.content);
		os << (listy.first ? '[' : '(');
		for (auto &a : listy.second) {
			if (!first) os << ' ';
			first = false;
			os << a;
		}
		os << (listy.first ? ']' : ')');
	}

	return os;
}

inline bool tokMatch(const std::string_view &src, Token tok, std::string to) {
	if (tok.len != to.size()) return false;
	return strncmp(&src[tok.pos], &to[tok.pos], tok.len) == 0;
}
inline bool tokMatch(const std::string_view &src, Token tok, char to) {
	if (tok.len != 1) return false;
	return src[tok.pos] == to;
}
inline bool tokIsGeneric(const std::string_view &src, Token tok) {
	return src[tok.pos] == '\'';
}

class Parser {
	std::string_view m_src;
	std::vector<Token> m_toks;
	size_t m_pos;

	inline bool atend() const {
		return m_pos >= m_toks.size();
	}
	inline std::optional<Token> peek() const {
		if (!atend()) return m_toks[m_pos];
		else return {};
	}
	inline std::optional<Token> advance() {
		auto res = peek();
		++m_pos;
		return res;
	}
	inline bool atgeneric() const {
		if (!atend()) return tokIsGeneric(m_src, m_toks[m_pos]);
		else return false;
	}
	inline bool match(std::string to) const {
		if (!atend()) return tokMatch(m_src, m_toks[m_pos], to);
		else return false;
	}
	inline bool match(char to) const {
		if (!atend()) return tokMatch(m_src, m_toks[m_pos], to);
		else return false;
	}

	std::vector<Ast> parseSeq(char open, char close) {
		std::vector<Ast> res;

		// TODO: proper error reporting
		if (!match(open)) throw std::exception();
		advance();

		while (!match(close) && !atend()) {
			auto ast = next();
			// TODO: proper error reporting
			if (!ast.has_value()) throw std::exception();
			res.push_back(ast.value());
		}

		// TODO: proper error reporting
		if (!match(close)) throw std::exception();
		advance();

		return res;
	}
public:
	Parser(std::string_view src, const std::vector<Token> toks)
		: m_src(src), m_toks(std::move(toks)), m_pos(0) {}

	std::optional<Ast> next() {
		if (atend()) return {};

		if (match('(')) {
			auto seq = parseSeq('(', ')');
			return Ast(false, std::move(seq));
		} else if (match('[')) {
			auto seq = parseSeq('[', ']');
			return Ast(true, std::move(seq));
		} else if (atgeneric()) {
			return Ast(true, advance().value().get_tok(m_src));
		} else {
			return Ast(false, advance().value().get_tok(m_src));
		}
	}
};

std::vector<Ast> parse(const std::string_view &src,
		       const std::vector<Token> &toks) {
	Parser parser(src, toks);
	std::vector<Ast> res;

	auto elem = parser.next();
	while (elem.has_value()) {
		res.push_back(elem.value());
		elem = parser.next();
	}

	return res;
}
