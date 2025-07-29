#ifndef TYPES_H
#define TYPES_H

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

enum TypeType {
	TT_PRIMITIVE,
	TT_LIST,
	TT_TRANSFORM,
	TT_GENERIC,
	TT_TYPE,
};

enum PrimitiveType {
	TT_INT,
	TT_BOOL,
};

std::ostream &operator<<(std::ostream &os, PrimitiveType prim);

class Type;
using TypeRef = std::shared_ptr<const Type>;

class Type {
	TypeType m_type;
	union Type_union {
		PrimitiveType primitive;
		std::vector<TypeRef> list;
		std::pair<TypeRef, TypeRef> transform;
		std::string generic;
		TypeRef ofType;

	public:
		Type_union() : primitive(TT_INT) {}
		Type_union(PrimitiveType primitive) : primitive(primitive) {}
		Type_union(std::vector<TypeRef> list)
			: list(std::move(list)) {}
		Type_union(std::pair<TypeRef, TypeRef> transform)
			: transform(transform) {}
		Type_union(std::string generic)
			: generic(std::move(generic)) {}
		Type_union(TypeRef ofType) : ofType(ofType) {}

		~Type_union() {}
	} t;
public:
	Type(PrimitiveType primitive);
	Type(std::vector<TypeRef> list);
	Type(std::initializer_list<TypeRef> list);
	Type(std::pair<TypeRef, TypeRef> transform);
	Type(TypeRef from, TypeRef to);
	Type(std::string genericName);
	Type(TypeRef ofType);

	~Type();

	TypeType getType() const;

	const PrimitiveType &getPrimitive() const;
	const std::vector<TypeRef> &getList() const;
	const std::pair<TypeRef, TypeRef> &getTransform() const;
	const std::string &getGeneric() const;
	const TypeRef &getTypeof() const;
};

std::ostream &operator<<(std::ostream &os, const Type &type);

#endif /* TYPES_H */
