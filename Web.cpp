#include "Web.h"
#include <string>
#include <curl/curl.h>

size_t AppendDataToStringCurlCallback(void *ptr, size_t size, size_t nmemb, void *vstring)
{
	std::string * pstring = (std::string*)vstring;
	pstring->append((char*)ptr, size * nmemb);
	return size * nmemb;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

std::string DownloadString(std::string url) {
	std::string body;
	
	CURL *curl_handle;
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, AppendDataToStringCurlCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &body);
	curl_easy_perform(curl_handle);
	curl_easy_cleanup(curl_handle);

	return body;
}

void SendHTTPPostRequest(std::string hostname, std::string payload) {
		CURL *curl_handle;
		CURLcode retVal;
		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();
		curl_easy_setopt(curl_handle, CURLOPT_URL, hostname.c_str()); //host
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, payload.c_str()); //payload
		retVal = curl_easy_perform(curl_handle);
		curl_easy_cleanup(curl_handle);
}

void DownloadFile(std::string url,std::string fileName) {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(fileName.c_str(),"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
}