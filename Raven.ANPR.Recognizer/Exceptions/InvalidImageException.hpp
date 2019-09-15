
#pragma once
#include <exception>

class InvalidImageException : std::exception
{
public:
	explicit InvalidImageException(char const* const message) : exception(message)
	{
		
	}
};
