#include "AppUpdater.h"
#include "Web.h"
#include <PugiXML\pugixml.hpp>
#include "ZipUtils/zip.h"
#include "ZipUtils/unzip.h"
#include <Windows.h>
#include <sstream>
#include "GLApp\MathTools.h" //Contains

AppUpdater::AppUpdater(std::string appName, int versionId, std::string configFile)
{
	applicationName = appName;
	currentVersionId = versionId;
	configFileName = configFile;
	foundUpdate = false;
	LoadConfig();
}

void AppUpdater::SaveConfig()
{
}

void AppUpdater::LoadConfig()
{
}

void AppUpdater::SetUserUpdatePreference(bool answer) {
	checkForUpdates = answer;
	appLaunchesWithoutAsking = -1; //Don't ask again
	SaveConfig();
}

void AppUpdater::SkipUpdate(const UpdateManifest& update)
{
	if (!Contains(skippedVersionIds,update.versionId))
		skippedVersionIds.push_back(update.versionId);
}

int AppUpdater::RequestUpdateCheck() {
	if (!checkForUpdates) {
		if (appLaunchesWithoutAsking == -1) {
			return ANSWER_ALREADYDECIDED;
		} else if (appLaunchesWithoutAsking >= 2) { //Third time launching app, time to ask if we can check for updates
			return ANSWER_ASKNOW;
		}
		else {
			return ANSWER_DONTASKYET;
		}
	}
	else {
		//Do check for updates
		updateThread = std::thread(&AppUpdater::PerformUpdateCheck, (AppUpdater*)this); //Launch parallel update-checking thread
		return ANSWER_ALREADYDECIDED;
	}
}

void AppUpdater::PerformUpdateCheck() {
	//Update checker

	//std::string url = "https://molflow.web.cern.ch/sites/molflow.web.cern.ch/files/autoupdate.xml"; //Update feed
	std::string body = DownloadString(remoteUrl);
	//Handle errors

	pugi::xml_document updateDoc;
	pugi::xml_parse_result result = updateDoc.load_string(body.c_str());
	//Handle errors

	if (userId == "default") {
		//First update check: generate random install identifier, like a browser cookie (4 alphanumerical characters)
		//It is generated based on the computer's network name and the logged in user name
		//FOR USER PRIVACY, THESE ARE NOT SENT TO GOOGLE ANALYTICS, JUST AN ANONYMOUS HASH
		//Should get the same hash even in case of subsequent installs

		GenerateUserId();
	}

	//std::string GoogleAnalyticsTrackingId = "UA-86802533-2";
	//std::string eventCategory = "updateCheck";

	std::stringstream payload;
	payload << "v=1&t=event&tid=" << googleAnalyticsTrackingId << "&cid=" << userId << "&ec=" << googleAnalyticsEventCategory << "&ea=" << applicationName << "_" << currentVersionId;
	SendHTTPPostRequest("http://www.google-analytics.com/collect", payload.str()); //Sends random app and version id for analytics. Also sends install id to count number of users
}

void AppUpdater::GenerateUserId(){
	char computerName[1024];
	char userName[1024];
	DWORD size = 1024;
	GetComputerName(computerName, &size);
	GetUserName(userName, &size);
	std::string id = computerName;
	id += "/";
	id += userName;

	//Create hash code from computer name
	//Source: http://www.cse.yorku.ca/~oz/hash.html
	size_t hashCode = 5381;
	int c;
	size_t index = 0;
	while (c = id.c_str()[index++]) {
		hashCode = ((hashCode << 5) + hashCode) + c; /* hash * 33 + c */
	}

	//Convert hash number to alphanumerical hash (base62)
	std::string alphaNum =
	"0123456789"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz";

	userId = "";
	while (hashCode > 0) {
		size_t dividend = (size_t)(hashCode / alphaNum.length());
		size_t remainder = hashCode - dividend*alphaNum.length();
		hashCode = dividend;
		userId = alphaNum[remainder] + userId;
	}
}

std::string AppUpdater::DownloadInstallUpdate(const UpdateManifest& update) {

	//Download the zipped new version to parent directory
	std::stringstream zipDest; zipDest << "..\\" << update.zipName;
	DownloadFile(update.zipUrl, zipDest.str());

	//Extract it to LOCAL directory (no way to directly extract to parent)
	HZIP hz = OpenZip(zipDest.str().c_str(), 0);
	ZIPENTRY ze; GetZipItem(hz, -1, &ze); int numitems = ze.index;
	// -1 gives overall information about the zipfile
	for (int zi = 0; zi<numitems; zi++)
	{
		ZIPENTRY ze; GetZipItem(hz, zi, &ze); // fetch individual details
		UnzipItem(hz, zi, ze.name);           // e.g. the item's name.
	}
	CloseZip(hz);

	//ZIP file not required anymore
	DeleteFile(zipDest.str().c_str());

	//Move extracted dir to parent dir
	std::stringstream folderDest; folderDest << "..\\" << update.folderName;
	MoveFileEx(update.folderName.c_str(), folderDest.str().c_str(), MOVEFILE_WRITE_THROUGH);

	//Copy current config file to new version's dir
	if (update.copyCfg) {
		for (auto copyFile : update.filesToCopy) {
			std::stringstream configDest; configDest << folderDest.str() << copyFile;
			CopyFile(copyFile.c_str(), folderDest.str().c_str(), false);
		}
	}

	return "Update installed succesfully";
	//Optionally run "..\app_name.exe" and exit
}

void AppUpdater::IncreaseSessionCount()
{
	if (!(appLaunchesWithoutAsking == -1)) appLaunchesWithoutAsking++;
}

std::tuple<bool, UpdateManifest, std::string> AppUpdater::GetResult()
{
	return std::tie(foundUpdate, latestUpdate, cumulativeChangeLog);
}
