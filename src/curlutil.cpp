#include "eleman/curlutil.h"

eleman::CurlException::CurlException(CURLcode code)
{
	message = "CurlException with code ";
	message += std::to_string(code);
	message += " (" + std::string(curl_easy_strerror(code)) + ")";
}

eleman::CurlException::~CurlException()
{

}

const char * eleman::CurlException::what() const throw()
{
	return message.c_str();
}


static size_t callback(void* data, size_t size, size_t nmemb, void *userp)
{
	std::string* response = (std::string*) userp;
	response->append((char*) data);
	return size * nmemb;
}


std::string eleman::curlRequest(std::string url, std::string userAgent)
{
	CURL* curl;
	curl = curl_easy_init();

	if(!curl) throw CurlException(CURLE_FAILED_INIT);

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());

#ifdef __ANDROID__
	// TODO Find universal solution for this !!!
// 	curl_easy_setopt(curl, CURLOPT_CAPATH, "/system/etc/security/cacerts");
// 	curl_easy_setopt(curl, CURLOPT_CAPATH, "/data/misc/keychain");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif


	std::string result;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

	CURLcode code = curl_easy_perform(curl);
	if(code != CURLE_OK) throw CurlException(code);

	curl_easy_cleanup(curl);

	return result;
}
