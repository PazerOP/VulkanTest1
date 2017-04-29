#pragma once

#include <memory>
#include <vector>

class utf8string final
{
public:

	typedef std::char_traits<char> traits_type;
	typedef char32_t value_type;
	typedef std::allocator<value_type> allocator_type;
	typedef std::allocator_traits<allocator_type>::size_type size_type;
	typedef std::allocator_traits<allocator_type>::difference_type difference_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::allocator_traits<allocator_type>::pointer pointer;
	typedef std::allocator_traits<allocator_type>::const_pointer;

	typedef std::vector<value_type, allocator_type> buffer_type;
	typedef buffer_type::iterator iterator;
	typedef buffer_type::const_iterator const_iterator;
	typedef buffer_type::reverse_iterator reverse_iterator;

private:
	buffer_type m_Buffer;
};