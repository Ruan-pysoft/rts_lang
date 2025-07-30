#include "lex.hpp"
#include "parse.hpp"

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
	for (auto ast_value : parse(src, std::move(tokenize(src)))) {
		std::cout << ast_value << '\n';
	}
}
