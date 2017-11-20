#include "AppUpdater.h"
#include "Web.h"
#include "ZipUtils/zip.h"
#include "ZipUtils/unzip.h"
#include <Windows.h>
#include <sstream>
#include "GLApp\MathTools.h" //Contains

AppUpdater::AppUpdater(const std::string& appName, const int& versionId, const std::string& configFile)
{
	applicationName = appName;
	currentVersionId = versionId;
	configFileName = configFile;
	foundUpdate = false;
	LoadConfig();
}

void AppUpdater::SaveConfig()
{
	xml_document configDoc;
	xml_node rootNode = configDoc.append_child("UpdaterConfigFile"); //XML specifications require a root node

	xml_node serverNode = rootNode.append_child("ServerConfig");
	serverNode.append_child("RemoteFeed").append_attribute("url") = feedUrl.c_str();
	serverNode.append_child("PublicWebsite").append_attribute("url") = publicWebsite.c_str();
	serverNode.child("PublicWebsite").append_attribute("downloadsPage") = publicDownloadsPage.c_str();
	serverNode.append_child("GoogleAnalytics").append_attribute("projectId") = googleAnalyticsTrackingId.c_str();
	serverNode.child("GoogleAnalytics").append_attribute("eventCategory") = googleAnalyticsEventCategory.c_str();

	xml_node localConfigNode = rootNode.append_child("LocalConfig");
	localConfigNode.append_child("Permission").append_attribute("checkForUpdates") = checkForUpdates;
	localConfigNode.child("Permission").append_attribute("appLaunchedBeforeAsking") = appLaunchedWithoutAsking;
	localConfigNode.child("Permission").append_attribute("askAfterNbLaunches") = askAfterNbLaunches;
	localConfigNode.append_child("Branch").append_attribute("name") = branchName.c_str();
	localConfigNode.append_child("GoogleAnalytics").append_attribute("cookie") = userId.c_str();
	xml_node skippedVerNode = localConfigNode.append_child("SkippedVersions");
	for (auto version : skippedVersionIds) {
		skippedVerNode.append_child("Version").append_attribute("id") = version;
	}

	configDoc.save_file(configFileName.c_str());
}

void AppUpdater::LoadConfig()
{
	xml_document loadXML;
	xml_parse_result configDoc = loadXML.load_file(configFileName.c_str());
	xml_node rootNode = loadXML.child("UpdaterConfigFile"); //XML specifications require a root node

	xml_node serverNode = rootNode.child("ServerConfig");
	feedUrl = serverNode.child("RemoteFeed").attribute("url").as_string();
	publicWebsite = serverNode.child("PublicWebsite").attribute("url").as_string();
	publicDownloadsPage = serverNode.child("PublicWebsite").attribute("downloadsPage").as_string();
	googleAnalyticsTrackingId = serverNode.child("GoogleAnalytics").attribute("projectId").as_string();
	googleAnalyticsEventCategory = serverNode.child("GoogleAnalytics").attribute("eventCategory").as_string();

	xml_node localConfigNode = rootNode.child("LocalConfig");
	checkForUpdates = localConfigNode.child("Permission").attribute("checkForUpdates").as_bool();
	appLaunchedWithoutAsking = localConfigNode.child("Permission").attribute("appLaunchedBeforeAsking").as_int();
	askAfterNbLaunches = localConfigNode.child("Permission").attribute("askAfterNbLaunches").as_int();
	branchName = localConfigNode.child("Branch").attribute("name").as_string();
	userId = localConfigNode.child("GoogleAnalytics").attribute("cookie").as_string();
	xml_node skippedVerNode = localConfigNode.child("SkippedVersions");
	for (auto version : skippedVerNode.children("Version")) {
		skippedVersionIds.push_back(version.attribute("id").as_int());
	}
}

void AppUpdater::SetUserUpdatePreference(bool answer) {
	checkForUpdates = answer;
	appLaunchedWithoutAsking = -1; //Don't ask again
	SaveConfig();
}

void AppUpdater::SkipUpdate(const UpdateManifest& update)
{
	if (!Contains(skippedVersionIds,update.versionId))
		skippedVersionIds.push_back(update.versionId);
}

int AppUpdater::RequestUpdateCheck() {
	if (appLaunchedWithoutAsking == -1) {
		if (checkForUpdates) updateThread = std::thread(&AppUpdater::PerformUpdateCheck, (AppUpdater*)this); //Launch parallel update-checking thread
			return ANSWER_ALREADYDECIDED;
	} else if (appLaunchedWithoutAsking >= askAfterNbLaunches) { //Third time launching app, time to ask if we can check for updates
			return ANSWER_ASKNOW;
	} else {
			return ANSWER_DONTASKYET;
	}
}

void AppUpdater::PerformUpdateCheck() {
	//Update checker
	if (checkForUpdates) { //One extra safeguard to ensure that we (still) have the permission
		//std::string url = "https://molflow.web.cern.ch/sites/molflow.web.cern.ch/files/autoupdate.xml"; //Update feed

		std::string body = DownloadString(feedUrl);
		//Handle errors

		pugi::xml_document updateDoc;
		pugi::xml_parse_result result = updateDoc.load_string(body.c_str());
		//Parse document and handle errors
		std::tie(foundUpdate, latestUpdate, cumulativeChangeLog) = AnalyzeFeed(updateDoc, currentVersionId, branchName);


		if (Contains({ "","not_set","default" }, userId)) {
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
}

std::tuple<bool, UpdateManifest, std::string> AppUpdater::AnalyzeFeed(const pugi::xml_node& updateDoc, const int& currentVersionId, const std::string& branchName)
{
	int latestVersionId = currentVersionId;
	std::stringstream cumulativeChangeLog;
	UpdateManifest latestUpdate;

	xml_node rootNode = updateDoc.child("UpdateFeed");
	for (auto branchNode : rootNode.child("Branches").children("Branch")) { //Look for a child with matching branch name
		std::string currentBranchName = branchNode.attribute("name").as_string();
		if (currentBranchName == branchName) {
			for (xml_node updateNode : branchNode.children("UpdateManifest")) {
				int versionId = updateNode.child("Version").attribute("id").as_int();
				if (!Contains(skippedVersionIds,versionId) && versionId > latestVersionId) {
					latestUpdate.versionId = versionId;
					latestUpdate.changeLog = updateNode.child_value("ChangeLog");
					latestUpdate.date = updateNode.child("Version").attribute("date").as_string();
					latestUpdate.filesToCopy.clear();
					for (xml_node fileNode : updateNode.child("FilesToCopy").children("File")) {
						latestUpdate.filesToCopy.push_back(fileNode.attribute("name").as_string());
					}
					latestUpdate.name = updateNode.child("Version").attribute("name").as_string();

					latestUpdate.zipUrl = updateNode.child("Content").attribute("zipUrl").as_string();
					latestUpdate.zipName = updateNode.child("Content").attribute("zipName").as_string();
					latestUpdate.folderName = updateNode.child("Content").attribute("folderName").as_string();

					cumulativeChangeLog << latestUpdate.name << " (released " << latestUpdate.date << ")\n\n" << latestUpdate.changeLog << "\n\n"; //No sorting: for a nice cumulative changelog, updates should be in chronological order (newest first)
					latestVersionId = versionId;
				}
			}
		}
	}
	bool foundUpdate = latestVersionId > currentVersionId;
	return std::tie(foundUpdate, latestUpdate, cumulativeChangeLog.str());
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
	//Hash algorithm source: http://www.cse.yorku.ca/~oz/hash.html
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
	for (auto copyFile : update.filesToCopy) {
		std::stringstream configDest; configDest << folderDest.str() << "\\" << copyFile;
		CopyFile(copyFile.c_str(), configDest.str().c_str(), false);
	}

	return "Update installed succesfully";
	//Optionally run "..\app_name.exe" and exit
}

void AppUpdater::IncreaseSessionCount()
{
	if (!(appLaunchedWithoutAsking == -1)) appLaunchedWithoutAsking++;
}

std::tuple<bool, UpdateManifest, std::string> AppUpdater::GetResult()
{
	return std::tie(foundUpdate, latestUpdate, cumulativeChangeLog);
}
