#ifndef CURLUTIL_H
#define CURLUTIL_H

#include <curl/curl.h>

#include <stdexcept>
#include <string>

namespace eleman
{

	class CurlException : public std::exception
	{
	public:
		/**
		 * Default constructor
		 */
		CurlException(CURLcode code);

		/**
		 * Destructor
		 */
		~CurlException();

		const char* what() const throw();
		CURLcode getCode() const { return code; };
		std::string getMessage() const { return message; };

	private:
		CURLcode code;
		std::string message;
	};

	std::string curlRequest(std::string url, std::string userAgent);

}	// end namespace eleman

#endif // CURLUTIL_H
