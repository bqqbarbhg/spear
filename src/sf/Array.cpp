#include "Array.h"
#include "Reflection.h"

namespace sf {

struct ArrayType final : Type
{
	ArrayType(Type *elemType)
		: Type("Array", sizeof(ArrayBase), HasArray)
	{
		elementType = elemType;
	}

	virtual void getName(sf::StringBuf &buf)
	{
		buf.append("Array<");
		elementType->getName(buf);
		buf.append(">");
	}

	virtual VoidSlice instGetArray(void *inst)
	{
		ArrayBase *arr = (ArrayBase*)inst;
		return { arr->data, arr->size };
	}

};

Type *initArrayType(Type *elemType)
{
	return new ArrayType(elemType);
}

}
