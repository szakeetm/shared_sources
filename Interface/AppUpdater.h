

/*
App updater methods and dialogs.
Adding it to a program requires the following:

----------On program startup----------
- Create a new instance of the AppUpdater class
- Call the RequestUpdateCheck() method
- If the return value is ANSWER_ASKNOW, display a dialog (for example the included UpdateCheckDialog) where the user can decide whether he wants update checks. Based on his answer call SetUserUpdatePreference(bool answer) method

----------On program exit-------------
- Call IncreaseSessionCount()

----------Periodically during the program run (for example in the main event loop)-----------
- Check for background update check process' result:
- IsUpdateAvailable() tells if an update was found
	- (Optional) Create a log window that has a public method Log(string message) to see the update process result. You can use the included UpdateLogWindow
	- If IsUpdateAvailable() is true, display a dialog (for example the included UpdateFoundDialog to ask whether to install that update)

	- If the users asks to install, call InstallLatestUpdate(), then ClearAvailableUpdates()
	- If he asks to skip the available update(s), call SkipAvailableUpdates(), then ClearAvailableUpdates();
	- If he asks to ask later, call ClearAvailableUpdates()
	- If he asks to disable update checking, call ClearAvailableUpdates() then SetUserUpdatePreference(false);

---------In program settings window------------
- Query update check setting by IsUpdateCheckAllowed()
- Set update check setting by SetUserUpdatePreference()

--------Shipped config file--------------------
<?xml version="1.0"?>
<UpdaterConfigFile>
	<ServerConfig>
		<RemoteFeed url="https://company.com/autoupdate.xml" />
		<PublicWebsite url="https://company.com/" downloadsPage="https://company.com/content/downloads" />
		<MatomoAnalytics siteId="123" requestTarget="https://acme.com/matomo.php" />
	</ServerConfig>
	<LocalConfig>
		<Permission allowUpdateCheck="false" appLaunchedBeforeAsking="0" askAfterNbLaunches="3" />
		<Branch name="appname_public" />
		<MatomoAnalytics userHash="not_set" />
		<SkippedVersions />
	</LocalConfig>
</UpdaterConfigFile>

appLaunchedBeforeAsking meaning:
default: 0
already asked: -1
increased at every session, except if already asked (-1)
compared with askAfterNbLaunches

------------Example implementation in Molflow/Synrad--------------
-----In Interface::OneTimeSceneInit_post()----------

appUpdater = new AppUpdater(appName, appVersionId, "updater_config.xml");
int answer = appUpdater->RequestUpdateCheck();
if (answer == ANSWER_ASKNOW) {
updateCheckDialog = new UpdateCheckDialog(appName, appUpdater);
updateCheckDialog->SetVisible(true);
wereEvents = true;

----In GlobalSettings-------------

if (mApp->appUpdater) { //Updater initialized
	chkCheckForUpdates->SetThreadStates(mApp->appUpdater->IsUpdateCheckAllowed());
}
//...
if (mApp->appUpdater) {
	if (mApp->appUpdater->IsUpdateCheckAllowed() != updateCheckPreference) {
		mApp->appUpdater->SetUserUpdatePreference(updateCheckPreference);
	}
}


---In interface constructor ----------
appUpdater = NULL; //We'll initialize later, when the app name and version id is known

---In Interface::AfterExit()-------------
if (appUpdater) {
	appUpdater->IncreaseSessionCount();
}


-----in Interface::FrameMove()-----------
//Check if app updater has found updates
if (appUpdater && appUpdater->IsUpdateAvailable()) {
	if (!updateLogWindow) {
		updateLogWindow = new UpdateLogWindow(this);
	}
	if (!updateFoundDialog) {
		updateFoundDialog = new UpdateFoundDialog(appName, appVersionName, appUpdater, updateLogWindow);
		updateFoundDialog->SetVisible(true);
		wereEvents = true;
		}
	}
}
*/

#pragma once
#include <string>
#include <vector>
#include <thread>
#include <tuple>
#include <PugiXML/pugixml.hpp>
using namespace pugi;
#include "GLApp/GLWindow.h"
#include "Interface/Interface.h" //DoEvents
#include <filesystem>
#include "Web.h"

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
	std::vector<std::pair<std::string, std::vector<std::string>>> postInstallScripts; //first component: comment, second component: vector of OS system commands
	std::vector<std::string> executableBinaries;
};

class GLButton;
class GLLabel;
class GLList;

class UpdateLogWindow : public GLWindow {
public:
	UpdateLogWindow(Interface* mApp);

	// Implementation
	void ProcessMessage(GLComponent *src, int message) override;
	void ClearLog();
	void Log(const std::string& line);
	void SetBounds(int x, int y, int w, int h) override;
private:
	void RebuildList();
	
	GLList *logList;
	GLButton *okButton,*copyButton;
	std::vector<std::string> lines;
	Interface* mApp;
	bool isLocked;
};

class AppUpdater;

class UpdateFoundDialog : public GLWindow {
public:
	UpdateFoundDialog(const std::string& appName, const std::string& appVersionName, AppUpdater* appUpdater, UpdateLogWindow* logWindow);

	// Implementation
	void ProcessMessage(GLComponent *src, int message);
private:
	GLLabel *questionLabel;
	GLButton *updateButton, *laterButton, *skipButton, *disableButton;
	AppUpdater* updater;
	UpdateLogWindow* logWnd;
};

class UpdateWarningDialog : public GLWindow {
public:
    UpdateWarningDialog(AppUpdater* appUpdater);

    // Implementation
    void ProcessMessage(GLComponent *src, int message);
private:
    GLLabel *questionLabel;
    GLButton *yesButton, *noButton;
    AppUpdater* updater;
};

class ManualUpdateCheckDialog : public GLWindow {
public:
    ManualUpdateCheckDialog(const std::string & appName, const std::string& appVersionName, AppUpdater* appUpdater, UpdateLogWindow* logWindow, UpdateFoundDialog* foundWindow);
    void Refresh();
    // Implementation
    void ProcessMessage(GLComponent *src, int message);
private:
    GLLabel *questionLabel;
    GLButton *updateButton;
    GLButton *cancelButton;
    std::string appName;
    std::string appVersionName;
    AppUpdater* updater;
    UpdateLogWindow* logWnd;
    UpdateFoundDialog* foundWnd;
};

enum FetchStatus
{
	NONE = 0,
	DOWNLOAD_ERROR = 1,
	OTHER_ERROR = 2, OKAY, PARSE_ERROR
};

class AppUpdater {
private:
    void MakeDefaultConfig();
public:
	AppUpdater(const std::string& appName, const int versionId, const std::string& configFile);
    ~AppUpdater(){
        SAFE_DELETE(updateWarning);
        if (updateThread.joinable()) {
            updateThread.join();
        }
    }
	bool IsUpdateAvailable();
	bool IsUpdateCheckAllowed() const;
	void ClearAvailableUpdates();
	std::string GetLatestUpdateName();
	std::string GetCumulativeChangeLog();
    std::string GetLatestChangeLog();

	int RequestUpdateCheck(); //Host app requesting update check, and is prepared to treat a possible "ask user if I can check for updates" dialog. Usually called on app startup. If we already have user permission, launches async updatecheck process
    void NotifyServerWarning();
    void AllowFurtherWarnings(bool allow);

    void PerformImmediateCheck();

	void SetUserUpdatePreference(bool answer);
	void SkipAvailableUpdates();
	void InstallLatestUpdate(UpdateLogWindow* logWindow);
	void IncreaseSessionCount();
	FetchStatus GetStatus(){return lastFetchStatus;};
	std::string GetPublicWebsiteAddress() { return publicWebsite; };
	int GetNbCheckFailsInRow() { return nbUpdateFailsInRow; };
private:

	//Initialized by constructor:
	int currentVersionId;
	FetchStatus lastFetchStatus = FetchStatus::NONE;
	std::string applicationName;
	std::string configFileName;

	//Initialized by shipped config file:
	std::string branchName,os;
	//std::string feedUrl; //Hard-code to avoid tampering
	std::string publicWebsite,publicDownloadsPage;
	MatomoTracker tracker;
	
	//Values that are generated during run
	std::vector<int> skippedVersionIds;
	std::thread updateThread;
	bool allowUpdateCheck;
	int appLaunchedWithoutAsking, askAfterNbLaunches; //Number of app launches before asking if user wants to check for updates. 0: default (shipping) value, -1: user already answered
    int nbUpdateFailsInRow, askAfterNbUpdateFails; // Number of app launches in a row, in that an update couldn't be fetched
	std::vector<UpdateManifest> availableUpdates; //empty in the beginning, populated upon update check

	//Methods
	void SaveConfig();
	void LoadConfig();
	void PerformUpdateCheck(bool forceCheck); //Actually check for updates (once we have user permission)


    std::vector<UpdateManifest> DetermineAvailableUpdates(const pugi::xml_node& updateFeed, const int currentVersionId);
    std::vector<UpdateManifest> DetermineAvailableUpdatesOldScheme(const pugi::xml_node& updateFeed, const int currentVersionId, const std::string& branchName);
	void DownloadInstallUpdate(const UpdateManifest& update, UpdateLogWindow *logWindow=NULL); //Download, unzip, move new version and copy config files. Return operation result as a user-readable message
	//void ExecutePostInstallScripts(const std::vector<std::pair<std::string, std::vector<std::string>>>& postInstallScripts, std::filesystem::path workingDir); //Async sys calls
	void GiveExecPermission(const std::string& binaryName); //throws error

	static UpdateManifest GetLatest(const std::vector<UpdateManifest>& updatesstatic );
    std::string GetCumulativeChangeLog(const std::vector<UpdateManifest>& updates);
    std::string GetLatestChangeLog(const std::vector<UpdateManifest>& updates);
    void SkipVersions(const std::vector<UpdateManifest>& updates);

	void GenerateUserId();
    UpdateWarningDialog* updateWarning;
};

class UpdateCheckDialog : public GLWindow {
public:
	UpdateCheckDialog(const std::string& appName, AppUpdater* appUpdater);

	// Implementation
	void ProcessMessage(GLComponent *src, int message);
private:
	GLLabel *questionLabel;
	GLButton *allowButton, *declineButton, *laterButton, *privacyButton;
	AppUpdater* updater;
};

#if defined(__LINUX_FEDORA) // set by cmake
#define BRANCH_OS_SUFFIX "_linux_fedora" //for old XML scheme
#define OS_ID "fedora" //for new XML scheme
#elif defined(__LINUX_DEBIAN) // set by cmake
#define BRANCH_OS_SUFFIX "_linux_debian"
#define OS_ID "debian"
#elif defined(WIN32) || defined(_WIN32) || (defined(__CYGWIN__) && defined(__x86_64__)) || defined(__MINGW32__)
#define BRANCH_OS_SUFFIX "_win"
#define OS_ID "win"
#elif (defined(__MACOSX__) || defined(__APPLE__)) && defined(__ARM_ARCH)
#define BRANCH_OS_SUFFIX "_mac_arm"
#define OS_ID "mac_arm"
#elif defined(__MACOSX__) || defined(__APPLE__)
#define BRANCH_OS_SUFFIX "_mac"
#define OS_ID "mac_intel"
#endif //BRANCH_OS_SUFFIX

#ifndef BRANCH_OS_SUFFIX
#error "No supported OS found for app updater config."
#endif