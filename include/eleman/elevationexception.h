#ifndef ELEVATIONEXCEPTION_H
#define ELEVATIONEXCEPTION_H

#include <stdexcept>
#include <string>

/**
 * @todo write docs
 */
class ElevationException : public std::exception
{
public:
    /**
     * Default constructor
	 */
	ElevationException();

	ElevationException(std::string message);

    /**
     * Destructor
     */
    ~ElevationException();

	const char* what() const throw();
	std::string getMessage() const;

private:
	std::string message;
};

#endif // ELEVATIONEXCEPTION_H
