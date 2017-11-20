#pragma once
#include <string>
#include <vector>
#include <thread>
#include <tuple>
#include <PugiXML\pugixml.hpp>
using namespace pugi;

#define ANSWER_DONTASKYET 1
#define ANSWER_ALREADYDECIDED 2
#define ANSWER_ASKNOW 3

class UpdateManifest {
public:
	std::string name; //Version name, like "Molflow 2.6 beta"
	std::string date; //Release date
	std::string changeLog; //Changes since the last published version
	int versionId; //Will be compared with appVersion

	std::string zipUrl; //URL of ZIP file to download
	std::string zipName; //Local download target fileName
	std::string folderName; //Folder name expected in ZIP file, also the unzipped program is moved to this sister folder
	std::vector<std::string> filesToCopy; //Config files to copy to new dir
};

class AppUpdater {
	
	public:
	AppUpdater(const std::string& appName, const int& versionId, const std::string& configFile);

	
	//Initialized by constructor:
	int currentVersionId;
	std::string applicationName;
	std::string configFileName;

	//Initialized by shipped config file:
	std::string branchName;
	std::string feedUrl,publicWebsite,publicDownloadsPage;
	std::string googleAnalyticsTrackingId, googleAnalyticsEventCategory;
	
	//Values that are generated during run
	std::string	userId; //User unique identifier. Default value: "not_set"
	std::vector<int> skippedVersionIds;
	std::thread updateThread;
	bool checkForUpdates;
	int appLaunchedWithoutAsking, askAfterNbLaunches; //Number of app launches before asking if user wants to check for updates. 0: default (shipping) value, -1: user already answered

	//Updatecheck result (state)
	bool foundUpdate;
	UpdateManifest latestUpdate;
	std::string cumulativeChangeLog; //diff between current version and latest

	//Methods
	void SaveConfig();
	void LoadConfig();
	void SetUserUpdatePreference(bool answer);
	void SkipUpdate(const UpdateManifest& update);

	//Communication with host app
	int RequestUpdateCheck(); //Host app requesting update check, and is prepared to treat a possible "ask user if I can check for updates" dialog. Usually called on app startup. If we already have user permission, launches async updatecheck process
	std::tuple<bool, UpdateManifest, std::string> GetResult(); //Host app querying async process result (whether there was an update). Usually called regularly, i.e. in the main event loop. Return values: isUpdateAvailable,latestVersionName(can include version Id and Date), cumulative changelog. If there was an update, host app should display a dialog whether user wants to update, skip this version, install later or disable update check

	void PerformUpdateCheck(); //Actually check for updates (once we have user permission)
	std::tuple<bool, UpdateManifest, std::string> AnalyzeFeed(const pugi::xml_node& updateDoc, const int& currentVersionId, const std::string& branchName);
	std::string DownloadInstallUpdate(const UpdateManifest& update); //Download, unzip, move new version and copy config files. Return operation result as a user-readable message
	
	void GenerateUserId();
	void IncreaseSessionCount();
};