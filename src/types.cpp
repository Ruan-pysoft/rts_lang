#include "types.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

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

Type::Type(PrimitiveType primitive) : m_type(TT_PRIMITIVE), t(primitive) {}
Type::Type(std::vector<TypeRef> list) : m_type(TT_LIST), t(std::move(list)) {}
Type::Type(std::initializer_list<TypeRef> list) : m_type(TT_LIST), t(list) {}
Type::Type(std::pair<TypeRef, TypeRef> transform)
	: m_type(TT_TRANSFORM), t(transform) {}
Type::Type(TypeRef from, TypeRef to)
	: m_type(TT_TRANSFORM), t(std::make_pair(from, to)) {}
Type::Type(std::string genericName)
	: m_type(TT_GENERIC), t(std::move(genericName)) {}
Type::Type(TypeRef ofType) : m_type(TT_TYPE), t(ofType) {}

Type::~Type() {
	switch (m_type) {
		case TT_PRIMITIVE: {
			t.primitive.~PrimitiveType();
		break; }
		case TT_LIST: {
			t.list.~vector();
		break; }
		case TT_TRANSFORM: {
			t.transform.~pair();
		break; }
		case TT_GENERIC: {
			t.generic.~basic_string();
		break; }
		case TT_TYPE: {
			t.ofType.~shared_ptr();
		}
	}
}

TypeType Type::getType() const {
	return m_type;
}

const PrimitiveType &Type::getPrimitive() const {
	if (m_type != TT_PRIMITIVE) {
		throw std::range_error("Tried accessing primitive value of non-primitive type");
	}

	return t.primitive;
}
const std::vector<TypeRef> &Type::getList() const {
	if (m_type != TT_LIST) {
		throw std::range_error("Tried accessing type list value of non-type list type");
	}

	return t.list;
}
const std::pair<TypeRef, TypeRef> &Type::getTransform() const {
	if (m_type != TT_TRANSFORM) {
		throw std::range_error("Tried accessing transform value of non-transform type");
	}

	return t.transform;
}
const std::string &Type::getGeneric() const {
	if (m_type != TT_GENERIC) {
		throw std::range_error("Tried accessing generic value of non-generic type");
	}

	return t.generic;
}
const TypeRef &Type::getTypeof() const {
	if (m_type != TT_TYPE) {
		throw std::range_error("Tried accessing subtype value of non-type type");
	}

	return t.ofType;
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
