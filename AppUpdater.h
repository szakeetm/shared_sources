#pragma once
#include <string>
#include <vector>
#include <thread>
#include <tuple>
#include <PugiXML\pugixml.hpp>
using namespace pugi;
#include "GLApp/GLWindow.h"

#define ANSWER_DONTASKYET 1
#define ANSWER_ALREADYDECIDED 2
#define ANSWER_ASKNOW 3

class UpdateManifest {
public:
	std::string name; //Version name, like "Molflow 2.6 beta"
	std::string date; //Release date
	std::string changeLog; //Changes since the last published version
	int versionId; //Will be compared with appVersionId

	std::string zipUrl; //URL of ZIP file to download
	std::string zipName; //Local download target fileName
	std::string folderName; //Folder name expected in ZIP file, also the unzipped program is moved to this sister folder
	std::vector<std::string> filesToCopy; //Config files to copy to new dir
};

class AppUpdater {
public:
	AppUpdater(const std::string& appName, const int& versionId, const std::string& configFile);

	bool IsUpdateAvailable();
	bool IsUpdateCheckAllowed();
	void ClearAvailableUpdates();
	std::string GetLatestUpdateName();
	std::string GetCumulativeChangeLog();

	int RequestUpdateCheck(); //Host app requesting update check, and is prepared to treat a possible "ask user if I can check for updates" dialog. Usually called on app startup. If we already have user permission, launches async updatecheck process

	void SetUserUpdatePreference(bool answer);
	void SkipAvailableUpdates();
	std::string InstallLatestUpdate();
	void IncreaseSessionCount();
	
private:

	//Initialized by constructor:
	int currentVersionId;
	std::string applicationName;
	std::string configFileName;

	//Initialized by shipped config file:
	std::string branchName;
	std::string feedUrl,publicWebsite,publicDownloadsPage;
	std::string googleAnalyticsTrackingId;
	
	//Values that are generated during run
	std::string	userId; //User unique identifier. Default value: "not_set"
	std::vector<int> skippedVersionIds;
	std::thread updateThread;
	bool allowUpdateCheck;
	int appLaunchedWithoutAsking, askAfterNbLaunches; //Number of app launches before asking if user wants to check for updates. 0: default (shipping) value, -1: user already answered

	std::vector<UpdateManifest> availableUpdates; //empty in the beginning, populated upon update check

	//Methods
	void SaveConfig();
	void LoadConfig();
	void PerformUpdateCheck(); //Actually check for updates (once we have user permission)
	
	std::vector<UpdateManifest> DetermineAvailableUpdates(const pugi::xml_node& updateFeed, const int& currentVersionId, const std::string& branchName);
	std::string DownloadInstallUpdate(const UpdateManifest& update); //Download, unzip, move new version and copy config files. Return operation result as a user-readable message
	
	UpdateManifest GetLatest(const std::vector<UpdateManifest>& updates);
	std::string GetCumulativeChangeLog(const std::vector<UpdateManifest>& updates);
	void SkipVersions(const std::vector<UpdateManifest>& updates);

	void GenerateUserId();
	
};

class GLButton;
class GLLabel;

class UpdateCheckDialog : public GLWindow {
public:
	UpdateCheckDialog(const std::string& appName, AppUpdater* appUpdater);

	// Implementation
	void ProcessMessage(GLComponent *src, int message);
private:
	GLLabel *questionLabel;
	GLButton *allowButton,*declineButton,*laterButton,*privacyButton;
	AppUpdater* updater;
};

class UpdateFoundDialog : public GLWindow {
public:
	UpdateFoundDialog(const std::string& appName, const std::string& appVersionName, AppUpdater* appUpdater);

	// Implementation
	void ProcessMessage(GLComponent *src, int message);
private:
	GLLabel *questionLabel;
	GLButton *updateButton, *laterButton, *skipButton, *disableButton;
	AppUpdater* updater;
};