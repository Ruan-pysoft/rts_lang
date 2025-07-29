#include "types.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <utility>
#include <variant>

std::ostream & operator<<(std::ostream &os, PrimitiveType prim) {
	switch (prim) {
		case TT_INT: {
			os << "int";
		break; }
		case TT_BOOL: {
			os << "bool";
		break; }
	}

	return os;
}

Type::Type(PrimitiveType primitive) : t(primitive) {}
Type::Type(std::vector<TypeRef> list) : t(std::move(list)) {}
Type::Type(std::initializer_list<TypeRef> list) : t(list) {}
Type::Type(std::pair<TypeRef, TypeRef> transform)
	: t(transform) {}
Type::Type(TypeRef from, TypeRef to)
	: t(std::make_pair(from, to)) {}
Type::Type(std::string genericName)
	: t(std::move(genericName)) {}
Type::Type(TypeRef ofType) : t(ofType) {}

TypeType Type::getType() const {
	if (std::holds_alternative<PrimitiveType>(t)) return TT_PRIMITIVE;
	else if (std::holds_alternative<ListType>(t)) return TT_LIST;
	else if (std::holds_alternative<TransformType>(t)) return TT_TRANSFORM;
	else if (std::holds_alternative<Generic>(t)) return TT_GENERIC;
	else if (std::holds_alternative<TypeRef>(t)) return TT_TYPE;
	else assert("This shouldbe unreachable!" && false);
}

const PrimitiveType &Type::getPrimitive() const {
	return std::get<PrimitiveType>(t);
}
const ListType &Type::getList() const {
	return std::get<ListType>(t);
}
const TransformType &Type::getTransform() const {
	return std::get<TransformType>(t);
}
const Generic &Type::getGeneric() const {
	return std::get<Generic>(t);
}
const TypeRef &Type::getTypeof() const {
	return std::get<TypeRef>(t);
}

std::ostream &operator<<(std::ostream &os, const Type &type) {
	switch (type.getType()) {
		case TT_PRIMITIVE: {
			os << type.getPrimitive();
		break; }
		case TT_LIST: {
			bool first = true;
			os << '(';
			for (auto &tp : type.getList()) {
				if (!first) os << ' ';
				first = false;
				os << *tp;
			}
			os << ')';
		break; }
		case TT_TRANSFORM: {
			auto &t = type.getTransform();
			os << *t.first << ' ' << *t.second << " ->";
		break; }
		case TT_GENERIC: {
			os << '\'' << type.getGeneric();
		break; }
		case TT_TYPE: {
			os << "type#" << *type.getTypeof();
		break; }
	}

	return os;
}
