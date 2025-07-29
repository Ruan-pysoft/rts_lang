#include "types.hpp"

#include <iostream>

int main() {
	auto Int = TypeRef(new Type(TT_INT));
	auto CondPost = TypeRef(new Type({
		TypeRef(new Type(TT_INT)),
		TypeRef(new Type(TT_BOOL))
	}));
	auto Cond = TypeRef(new Type(Int, CondPost));
	auto Gen = TypeRef(new Type("a"));
	auto IntType = TypeRef(new Type(Int));

	std::cout << *Int << std::endl;
	std::cout << *CondPost << std::endl;
	std::cout << *Cond << std::endl;
	std::cout << *Gen << std::endl;
	std::cout << *IntType << std::endl;
}
