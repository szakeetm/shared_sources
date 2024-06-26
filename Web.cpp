
#include "Web.h"
#include <fmt/core.h>
#include <string>

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

std::tuple<CURLcode,std::string> DownloadString(const std::string& url) {
	std::string body;
	
	CURL *curl_handle;
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, AppendDataToStringCurlCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &body);
	CURLcode result = curl_easy_perform(curl_handle);
	curl_easy_cleanup(curl_handle);

	return { result,body };
}

CURLcode SendHTTPPostRequest(const std::string& hostname, const std::string& payload) {
	CURL *curl_handle;
	CURLcode retVal;
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, hostname.c_str()); //host
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, payload.c_str()); //payload
	std::cout << "CURL http request result (ascii or binary): ";
	retVal = curl_easy_perform(curl_handle);
	std::cout << std::endl;
	curl_easy_cleanup(curl_handle);
	return retVal;
}

std::tuple<CURLcode, long> DownloadFile(const std::string& url, const std::string& fileName) {
	CURL* curl;
	FILE* fp;
	CURLcode result;
	long respCode;
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(fileName.c_str(), "wb");
		if (fp == NULL) return { CURLE_WRITE_ERROR,0 };
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		result = curl_easy_perform(curl);
		if (result == CURLE_OK) {
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respCode);
		}
		else {
			respCode = 0;
		}
		curl_easy_cleanup(curl);
		fclose(fp);
		return { result,respCode };
	}
	else return {CURLE_FAILED_INIT, 0};
}

CURLcode MatomoTracker::Send(const MatomoHttpRequest& request, const ScreenSize& size) {
	std::string payload = fmt::format("idsite={}&rec=1",FormatHttpString(siteId));
	if (!userId.empty()) payload += fmt::format("&uid={}",userId);
	if (size.width!=0 && size.height!=0) payload += fmt::format("&res={}x{}",size.width,size.height);

	if (!request.eventCategory.empty()) payload += fmt::format("&e_c={}",FormatHttpString(request.eventCategory));
	if (!request.eventAction.empty()) payload += fmt::format("&e_a={}",FormatHttpString(request.eventAction));
	if (!request.eventName.empty()) payload += fmt::format("&e_n={}",FormatHttpString(request.eventName));
	if (request.eventValue!=0.0) payload += fmt::format("&e_v={}",request.eventValue);

	int dimId = 1;
	for (int i = 0; i < persistentCustomDimensions.size(); i++) {
		if (!persistentCustomDimensions[i].empty()) payload += fmt::format("&dimension{}={}", dimId++, FormatHttpString(persistentCustomDimensions[i]));
	}
	for (int i=0;i<request.customDimensions.size();i++) {
		if (!request.customDimensions[i].empty()) payload += fmt::format("&dimension{}={}", dimId++,FormatHttpString(request.customDimensions[i]));
	}

	return SendHTTPPostRequest(requestTarget,payload);
}

std::string FormatHttpString(std::string str) {
    std::string replaceStr = "%20";
    std::string space = " ";

    size_t pos = 0;
    while ((pos = str.find(space, pos)) != std::string::npos) {
        str.replace(pos, space.length(), replaceStr);
        pos += replaceStr.length();
    }

    return str;
}