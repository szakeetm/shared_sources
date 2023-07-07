#include <curl/curl.h>
#include <string>
#include <tuple>
#include <vector>
#include "GLApp/GLToolkit.h" //ScreenSize

size_t AppendDataToStringCurlCallback(void *ptr, size_t size, size_t nmemb, void *vstring);

std::tuple<CURLcode,std::string> DownloadString(const std::string& url);
CURLcode SendHTTPPostRequest(const std::string& hostname, const std::string& payload);
std::tuple<CURLcode,long> DownloadFile(const std::string& url, const std::string& fileName);

struct MatomoHttpRequest {
    std::string eventCategory,eventAction,eventName;
    double eventValue=0.0;
    std::vector<std::string> customDimensions;
};

struct MatomoTracker {
    std::string siteId,requestTarget,userId;
    std::vector<std::string> persistentCustomDimensions;
    CURLcode Send(const MatomoHttpRequest& request, const ScreenSize& size=ScreenSize(0,0));
};



std::string FormatHttpString(std::string str);