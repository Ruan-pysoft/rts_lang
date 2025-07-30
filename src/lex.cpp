#include "lex.hpp"

#include <cctype>
#include <optional>
#include <string>

std::string_view Token::get_tok(const std::string_view &str) const {
	return std::string_view(&str[pos], len);
}

class Reader {
	std::string_view m_src;
	size_t m_pos;

	inline bool atend() const {
		return m_pos >= m_src.size();
	}
	inline std::optional<char> peek() const {
		if (!atend()) return m_src[m_pos];
		else return {};
	}
	inline std::optional<char> advance() {
		++m_pos;
		return peek();
	}
	inline bool atspace() {
		if (!atend()) return isspace(m_src[m_pos]);
		else return false;
	}
	inline bool atspecial() {
		if (!atend()) {
			const char c = m_src[m_pos];
			return c == '(' || c == ')' || c == '[' || c == ']'
				|| c == '$';
		} else return false;
	}
	void skip_ws() {
		while (std::isspace(peek().value_or('.'))) {
			++m_pos;
		}
	}

public:
	Reader(std::string_view src) : m_src(src), m_pos(0) {}

	std::optional<Token> next() {
		skip_ws();

		if (atend()) return {};

		if (atspecial()) {
			const size_t here = m_pos;
			advance();
			return Token(here, 1);
		} else {
			const size_t start = m_pos;
			while (!atspace() && !atspecial()) advance();
			return Token(start, m_pos - start);
		}
	}
};

std::vector<Token> tokenize(const std::string_view &str) {
	Reader reader(str);
	std::vector<Token> res;

	auto elem = reader.next();
	while (elem.has_value()) {
		res.push_back(elem.value());
		elem = reader.next();
	}

	return res;
}
