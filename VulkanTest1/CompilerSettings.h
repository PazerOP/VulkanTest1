#pragma once

#pragma warning(disable : 4061)		// enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(disable : 4201)		// nonstandard extension used : nameless struct/union
#pragma warning(disable : 4267)		// 'var' : conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable : 4365)		// 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#pragma warning(disable : 4464)		// relative include path contains '..'
#pragma warning(disable : 4514)		// 'function': unreferenced inline function has been removed
#pragma warning(disable : 4571)		// Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#pragma warning(disable : 4582)		// '%$S': constructor is not implicitly called
#pragma warning(disable : 4583)		// '%$S': destructor is not implicitly called
#pragma warning(disable : 4623)		// 'derived class': default constructor was implicitly defined as deleted because a base class default constructor is inaccessible or deleted
#pragma warning(disable : 4625)		// 'derived class': copy constructor was implicitly defined as deleted because a base class copy constructor is inaccessible or deleted
#pragma warning(disable : 4626)		// 'derived class': assignment operator was implicitly defined as deleted because a base class assignment operator is inaccessible or deleted
#pragma warning(disable : 4668)		// 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directive'
#pragma warning(disable : 4710)		// 'function': function not inlined
#pragma warning(disable : 4714)		// function 'function' marked as __forceinline not inlined
#pragma warning(disable : 4774)		// ‘<function>’ : format string expected in argument <position> is not a string literal
#pragma warning(disable : 4820)		// 'bytes' bytes padding added after construct 'member_name'
#pragma warning(disable : 4987)		// nonstandard extension used: 'throw (...)'
#pragma warning(disable : 5026)
#pragma warning(disable : 5027)

#pragma warning(error : 4172)		// returning address of local variable or temporary
#pragma warning(error : 4238)		// nonstandard extension used: class rvalue used as lvalue
#pragma warning(error : 4566)		// character represented by universal-character-name 'char' cannot be represented in the current code page (%d)



#define VK_USE_PLATFORM_WIN32_KHR