#ifndef TYPES_H
#define TYPES_H

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <variant>
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

using ListType = std::vector<TypeRef>;
using TransformType = std::pair<TypeRef, TypeRef>;
using Generic = std::string;

class Type {
	std::variant<PrimitiveType, ListType, TransformType, Generic, TypeRef> t;
public:
	Type(PrimitiveType primitive);
	Type(ListType list);
	Type(std::initializer_list<TypeRef> list);
	Type(TransformType transform);
	Type(TypeRef from, TypeRef to);
	Type(Generic genericName);
	Type(TypeRef ofType);

	TypeType getType() const;

	const PrimitiveType &getPrimitive() const;
	const ListType &getList() const;
	const TransformType &getTransform() const;
	const Generic &getGeneric() const;
	const TypeRef &getTypeof() const;
};

std::ostream &operator<<(std::ostream &os, const Type &type);

#endif /* TYPES_H */
