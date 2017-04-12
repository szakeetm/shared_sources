#include <string>

size_t AppendDataToStringCurlCallback(void *ptr, size_t size, size_t nmemb, void *vstring);

std::string DownloadString(std::string url);
void SendHTTPPostRequest(std::string hostname, std::string payload);
void DownloadFile(std::string url,std::string fileName);