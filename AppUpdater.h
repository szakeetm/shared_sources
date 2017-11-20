#pragma once
#include <string>
#include <vector>
#include <thread>
#include <tuple>

#define ANSWER_DONTASKYET 1
#define ANSWER_ALREADYDECIDED 2
#define ANSWER_ASKNOW 3

class UpdateManifest {
public:
	std::string name;
	std::string changeLog;
	int versionId;

	std::string zipUrl;
	std::string zipName;
	std::string folderName;
	std::vector<std::string> filesToCopy;
	bool copyCfg;
};

class AppUpdater {
	
	public:
		AppUpdater(std::string appName, int versionId, std::string configFileName);
	bool     checkForUpdates;
	int      appLaunchesWithoutAsking; //Number of app launches before asking if user wants to check for updates. 0: default (shipping) value, -1: user already answered
	
	int currentVersionId;
	std::string applicationName;
	
	std::string	userId; //User unique identifier. Default value: "default"
	std::vector<int> skippedVersionIds;
	std::string branch;
	std::thread updateThread;
	std::string remoteUrl;
	std::string remoteBranch;
	std::string configFileName;

	std::string googleAnalyticsTrackingId;
	std::string googleAnalyticsEventCategory;
	
	bool foundUpdate;
	UpdateManifest latestUpdate;
	std::string cumulativeChangeLog;

	void SaveConfig();
	void LoadConfig();
	void SetUserUpdatePreference(bool answer);
	void SkipUpdate(const UpdateManifest& update);

	int RequestUpdateCheck();
	void PerformUpdateCheck();
	void GenerateUserId();
	std::string DownloadInstallUpdate(const UpdateManifest& update);
	void IncreaseSessionCount();

	std::tuple<bool,UpdateManifest,std::string> GetResult(); //Interface asking if there was an update. Return values: isUpdateAvailable,latestVersionName(can include version Id and Date), changelog

};