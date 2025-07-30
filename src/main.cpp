#include "lex.hpp"

#include <iostream>

const auto src =
	"[rot rot] ('a 'b 'c) ('c 'a 'b) -> : unrot =\n"
	"[\n"
	"  0 1 (int int) :\n"
	"  [rot 0 > swp pop] (int int int) (int int int bool) -> :\n"
	"  [unrot dup rot +] (int int int) (int int int) -> :\n"
	"  while\n"
	"  (int int int) :\n"
	"  swp pop swp pop\n"
	"] int int -> : fib =\n"
	"10 fib\n"
;

int main() {
	for (auto tok : tokenize(src)) {
		std::cout << tok.get_tok(src) << '\n';
	}
}
