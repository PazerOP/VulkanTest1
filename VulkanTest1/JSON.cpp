#include "stdafx.h"
#include "JSON.h"

namespace JSON
{
	static int TestFunc()
	{
		Value asdf;
		asdf = "hello"s;

		return 5;
	}
	static int test = TestFunc();

	internal::PlaceholderArray::PlaceholderArray()
	{
		new (placeholder) RealArray();
	}
	internal::PlaceholderArray::PlaceholderArray(const PlaceholderArray& other)
	{
		RealArray* realThis = reinterpret_cast<RealArray*>(placeholder);
		const RealArray* realOther = reinterpret_cast<const RealArray*>(other.placeholder);
		*realThis = RealArray(*realOther);
	}
	internal::PlaceholderArray::PlaceholderArray(PlaceholderArray&& other)
	{
		RealArray* realThis = reinterpret_cast<RealArray*>(placeholder);
		RealArray* realOther = reinterpret_cast<RealArray*>(other.placeholder);
		*realThis = RealArray(std::move(*realOther));
	}
	internal::PlaceholderArray::~PlaceholderArray()
	{
		RealArray* real = reinterpret_cast<RealArray*>(placeholder);
		real->~RealArray();
	}

	internal::PlaceholderObject::PlaceholderObject()
	{
		new (placeholder) RealObject();
	}
	internal::PlaceholderObject::PlaceholderObject(const PlaceholderObject& other)
	{
		RealObject* realThis = reinterpret_cast<RealObject*>(placeholder);
		const RealObject* realOther = reinterpret_cast<const RealObject*>(other.placeholder);
		*realThis = RealObject(*realOther);
	}
	internal::PlaceholderObject::PlaceholderObject(PlaceholderObject&& other)
	{
		RealObject* realThis = reinterpret_cast<RealObject*>(placeholder);
		RealObject* realOther = reinterpret_cast<RealObject*>(other.placeholder);
		*realThis = RealObject(std::move(*realOther));
	}
	internal::PlaceholderObject::~PlaceholderObject()
	{
		RealObject* real = reinterpret_cast<RealObject*>(placeholder);
		real->~RealObject();
	}

	Value LoadFromString(const std::string& str)
	{
		return Value();
	}
}

