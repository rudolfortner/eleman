#include "../include/eleman/elevationexception.h"

ElevationException::ElevationException()
{

}

ElevationException::ElevationException(std::string message)
{
	this->message = message;
}

ElevationException::~ElevationException()
{

}

const char * ElevationException::what() const throw()
{
	return message.c_str();
}

std::string ElevationException::getMessage() const
{
	return message;
}
