#include "AppUpdater.h"
#include "Web.h"
#include "ZipUtils/zip.h"
#include "ZipUtils/unzip.h"
#include <Windows.h>
#include <sstream>
#include "GLApp\MathTools.h" //Contains

#include "GLApp/GLToolkit.h"
//#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"

AppUpdater::AppUpdater(const std::string& appName, const int& versionId, const std::string& configFile)
{
	applicationName = appName;
	currentVersionId = versionId;
	configFileName = configFile;
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

	xml_node localConfigNode = rootNode.append_child("LocalConfig");
	localConfigNode.append_child("Permission").append_attribute("allowUpdateCheck") = allowUpdateCheck;
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

	xml_node localConfigNode = rootNode.child("LocalConfig");
	allowUpdateCheck = localConfigNode.child("Permission").attribute("allowUpdateCheck").as_bool();
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
	allowUpdateCheck = answer;
	appLaunchedWithoutAsking = -1; //Don't ask again
	SaveConfig();
}

void AppUpdater::SkipAvailableUpdates()
{
	SkipVersions(availableUpdates);
	SaveConfig();
}

std::string AppUpdater::InstallLatestUpdate()
{
	return DownloadInstallUpdate(GetLatest(availableUpdates));
}

void AppUpdater::SkipVersions(const std::vector<UpdateManifest>& updates)
{
	for (auto update : updates) {
		if (!Contains(skippedVersionIds, update.versionId)) {
			skippedVersionIds.push_back(update.versionId);
		}
	}
}

int AppUpdater::RequestUpdateCheck() {
	if (appLaunchedWithoutAsking == -1) {
		if (allowUpdateCheck) updateThread = std::thread(&AppUpdater::PerformUpdateCheck, (AppUpdater*)this); //Launch parallel update-checking thread
		return ANSWER_ALREADYDECIDED;
	}
	else if (appLaunchedWithoutAsking >= askAfterNbLaunches) { //Third time launching app, time to ask if we can check for updates
		return ANSWER_ASKNOW;
	}
	else {
		return ANSWER_DONTASKYET;
	}
}

void AppUpdater::PerformUpdateCheck() {
	//Update checker
	if (allowUpdateCheck) { //One extra safeguard to ensure that we (still) have the permission
		//std::string url = "https://molflow.web.cern.ch/sites/molflow.web.cern.ch/files/autoupdate.xml"; //Update feed

		std::string resultCategory;
		std::stringstream resultDetail;

		CURLcode downloadResult;
		std::string body;
		std::tie(downloadResult, body) = DownloadString(feedUrl);
		//Handle errors
		if (downloadResult == CURLE_OK) {

			pugi::xml_document updateDoc;
			pugi::xml_parse_result parseResult = updateDoc.load_string(body.c_str());
			//Parse document and handle errors

			if (parseResult.status == status_ok) { //parsed successfully
				availableUpdates = DetermineAvailableUpdates(updateDoc, currentVersionId, branchName);
				resultCategory = "updateCheck";
				resultDetail << "updateCheck_" << applicationName << "_" << currentVersionId;
			}
			else { //parse error
				resultCategory = "parseError";
				resultDetail << "parseError_" << parseResult.status << "_" << applicationName << "_" << currentVersionId;
			}
		}
		else { //download error
			resultCategory = "stringDownloadError";
			resultDetail << "stringDownloadError_" << downloadResult << "_" << applicationName << "_" << currentVersionId;
		}
		//Send result for analytics

		if (Contains({ "","not_set","default" }, userId)) {
			//First update check: generate random install identifier, like a browser cookie (4 alphanumerical characters)
			//It is generated based on the computer's network name and the logged in user name
			//FOR USER PRIVACY, THESE ARE NOT SENT TO GOOGLE ANALYTICS, JUST AN ANONYMOUS HASH
			//Should get the same hash even in case of subsequent installs

			GenerateUserId();
		}

		std::stringstream payload;
		payload << "v=1&t=event&tid=" << googleAnalyticsTrackingId << "&cid=" << userId << "&ec=" << resultCategory << "&ea=" << resultDetail.str();
		SendHTTPPostRequest("http://www.google-analytics.com/collect", payload.str()); //Sends random app and version id for analytics. Also sends install id to count number of users
	}
}

UpdateManifest AppUpdater::GetLatest(const std::vector<UpdateManifest>& updates) {
	int maxVersion = 0;
	size_t maxIndex = 0;
	for (size_t i = 0; i < updates.size(); i++) {
		if (updates[i].versionId > maxVersion) {
			maxVersion = updates[i].versionId;
			maxIndex = i;
		}
	}
	return updates[maxIndex];
}

std::vector<UpdateManifest> AppUpdater::DetermineAvailableUpdates(const pugi::xml_node& updateDoc, const int& currentVersionId, const std::string& branchName)
{
	std::vector<UpdateManifest> availableUpdates;

	xml_node rootNode = updateDoc.child("UpdateFeed");
	for (auto branchNode : rootNode.child("Branches").children("Branch")) { //Look for a child with matching branch name
		std::string currentBranchName = branchNode.attribute("name").as_string();
		if (currentBranchName == branchName) {
			for (xml_node updateNode : branchNode.children("UpdateManifest")) {
				int versionId = updateNode.child("Version").attribute("id").as_int();
				if (!Contains(skippedVersionIds, versionId) && versionId > currentVersionId) {
					UpdateManifest newUpdate;
					newUpdate.versionId = versionId;
					newUpdate.name = updateNode.child("Version").attribute("name").as_string();
					newUpdate.date = updateNode.child("Version").attribute("date").as_string();
					newUpdate.changeLog = updateNode.child_value("ChangeLog");

					newUpdate.zipUrl = updateNode.child("Content").attribute("zipUrl").as_string();
					newUpdate.zipName = updateNode.child("Content").attribute("zipName").as_string();
					newUpdate.folderName = updateNode.child("Content").attribute("folderName").as_string();

					for (xml_node fileNode : updateNode.child("FilesToCopy").children("File")) {
						newUpdate.filesToCopy.push_back(fileNode.attribute("name").as_string());
					}
					availableUpdates.push_back(newUpdate);
				}
			}
		}
	}
	return availableUpdates;
}

std::string AppUpdater::GetCumulativeChangeLog(const std::vector<UpdateManifest>& updates) {
	//No sorting: for a nice cumulative changelog, updates should be in chronological order (newest first)
	std::stringstream cumulativeChangeLog;
	for (auto update : updates) {
		cumulativeChangeLog << "Changes in version " << update.name << " (released " << update.date << "):\n" << update.changeLog << "\n";
	}
	return cumulativeChangeLog.str();
}

void AppUpdater::GenerateUserId() {
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

	std::stringstream payload;
	payload << "v=1&t=event&tid=" << googleAnalyticsTrackingId << "&cid=" << userId << "&ec=" << "updateStart" << "&ea=" << "updateStart_" << applicationName << "_" << currentVersionId << "_to_" << update.versionId;
	SendHTTPPostRequest("http://www.google-analytics.com/collect", payload.str()); //Sends random app and version id for analytics. Also sends install id to count number of users

	std::string resultCategory;
	std::stringstream resultDetail;
	std::stringstream userResult; //Reported to the user

	//Download the zipped new version to parent directory
	std::stringstream zipDest; zipDest << "..\\" << update.zipName;
	CURLcode dlResult = DownloadFile(update.zipUrl, zipDest.str());
	if (dlResult == CURLE_OK) { //Download success
		userResult << "Downloaded\n" << update.zipUrl << "\nto\n" << zipDest.str() << "\n\n";

		//Extract it to LOCAL directory (no way to directly extract to parent)
		HZIP hz = OpenZip(zipDest.str().c_str(), 0);
		ZIPENTRY ze;
		ZRESULT zipResult = GetZipItem(hz, -1, &ze);
		int numitems = ze.index;
		if (zipResult == ZR_OK) {
			userResult << update.zipName << " opened, it contains " << numitems << "files.\n\n";

			// -1 gives overall information about the zipfile
			for (int zi = 0; zi < numitems; zi++)
			{
				ZIPENTRY ze;
				zipResult = GetZipItem(hz, zi, &ze); // fetch individual details
				if (zipResult == ZR_OK) {
					UnzipItem(hz, zi, ze.name);           // e.g. the item's name.
				}
				else {
					resultCategory = "zipItemError";
					resultDetail << "zipItemError_" << zipResult << "_item_" << zi << "_" << applicationName << "_" << currentVersionId;
					userResult << "Couldn't open item " << zi << " of " << update.zipName << "\nUpdate process aborted.\n";
					break;
				}
			}
			CloseZip(hz);

			if (zipResult == ZR_OK) {
				userResult << "All files extracted.\n\n";

				//ZIP file not required anymore
				if (DeleteFile(zipDest.str().c_str()) == 0) {
					resultCategory = "zipDeleteError";
					resultDetail << "zipDeleteError_" << applicationName << "_" << currentVersionId;
					userResult << "Couldn't delete " << update.zipName << "\nUpdate process aborted.\n";
				}
				else {
					userResult << update.zipName << " deleted.\n\n";
					//Move extracted dir to parent dir
					std::stringstream folderDest; folderDest << "..\\" << update.folderName;
					if (MoveFileEx(update.folderName.c_str(), folderDest.str().c_str(), MOVEFILE_WRITE_THROUGH) == 0) {
						resultCategory = "folderMoveError";
						resultDetail << "folderMoveError_" << applicationName << "_" << currentVersionId;
						userResult << "Couldn't move\n" << update.folderName << "\nto\n" << folderDest.str() << "\nUpdate process aborted.\n";
					}
					else {
						userResult << "Moved the extracted folder\n" << update.folderName << "\nto\n" << folderDest.str() << "\n\n";
						//Copy current config file to new version's dir
						for (auto copyFile : update.filesToCopy) {
							std::stringstream configDest; configDest << folderDest.str() << "\\" << copyFile;
							if (CopyFile(copyFile.c_str(), configDest.str().c_str(), false) == 0) {
								resultCategory = "fileCopyWarning";
								resultDetail << "fileCopyWarning_" << copyFile << "_" << applicationName << "_" << currentVersionId;
								userResult << "Couldn't copy\n" << copyFile << "\nto\n" << configDest.str() << "\nFile skipped.\n";
								
								std::stringstream payload2;
								payload2 << "v=1&t=event&tid=" << googleAnalyticsTrackingId << "&cid=" << userId << "&ec=" << resultCategory << "&ea=" << resultDetail.str();
								SendHTTPPostRequest("http://www.google-analytics.com/collect", payload2.str()); //Sends random app and version id for analytics. Also sends install id to count number of users
							}
							else {
								userResult << "Copied the file\n" << copyFile << "\nto\n" << configDest.str() << "\n\n";
							}
						}
						resultCategory = "updateSuccess";
						resultDetail << "updateSuccess_" << applicationName << "_" << currentVersionId << "_to_" <<update.versionId;
						userResult << "Update successful. If you wish, you can now close this version\nand launch the new one in the adjacent " << folderDest.str() << " folder.\n";
					}
				}
			}
		}
		else {
			CloseZip(hz);
			resultCategory = "zipItemError";
			resultDetail << "zipItemError_" << zipResult << "_item_-1_" << applicationName << "_" << currentVersionId;
			userResult << "Couldn't open " << update.zipName << "\nUpdate process aborted.";
		}
	}
	else {
		resultCategory = "zipDownloadError";
		resultDetail << "zipDownloadError_" << dlResult << "_" << applicationName << "_" << currentVersionId;
		userResult << "Couldn't download \n" << update.zipUrl << "\nto\n" << zipDest.str() << "\nUpdate process aborted.";
	}

	std::stringstream payload3;
	payload3 << "v=1&t=event&tid=" << googleAnalyticsTrackingId << "&cid=" << userId << "&ec=" << resultCategory << "&ea=" << resultDetail.str();
	SendHTTPPostRequest("http://www.google-analytics.com/collect", payload3.str()); //Sends random app and version id for analytics. Also sends install id to count number of users

	return userResult.str();
	//Optionally run "..\app_name.exe" and exit
}

void AppUpdater::IncreaseSessionCount()
{
	if (!(appLaunchedWithoutAsking == -1)) {
		appLaunchedWithoutAsking++;
		SaveConfig();
	}
}

bool AppUpdater::IsUpdateAvailable()
{
	return (availableUpdates.size() > 0);
}

bool AppUpdater::IsUpdateCheckAllowed()
{
	return allowUpdateCheck;
}

void AppUpdater::ClearAvailableUpdates()
{
	availableUpdates.clear();
}

std::string AppUpdater::GetLatestUpdateName()
{
	std::stringstream name;
	UpdateManifest& latestUpdate = GetLatest(availableUpdates);
	name << latestUpdate.name << " (released " << latestUpdate.date << ")";
	return name.str();
}

std::string AppUpdater::GetCumulativeChangeLog()
{
	return GetCumulativeChangeLog(availableUpdates);
}

UpdateCheckDialog::UpdateCheckDialog(const std::string & appName, AppUpdater* appUpdater)
{
	updater = appUpdater;

	int wD = 325;
	int hD = 95;


	std::stringstream question;
	question << "Would you like " << appName << " to check for updates on startup?\n(Change this later in Global Settings.)";

	questionLabel = new GLLabel(question.str().c_str());
	questionLabel->SetBounds(5, 5, 150, 60);
	Add(questionLabel);

	allowButton = new GLButton(0, "Yes, check");
	allowButton->SetBounds(5, hD - 45, 100, 19);
	Add(allowButton);

	declineButton = new GLButton(0, "Don't check");
	declineButton->SetBounds(110, hD - 45, 100, 19);
	Add(declineButton);

	laterButton = new GLButton(0, "Ask later");
	laterButton->SetBounds(215, hD - 45, 100, 19);
	Add(laterButton);

	privacyButton = new GLButton(0, "Privacy");
	privacyButton->SetBounds(215, hD - 70, 100, 19);
	Add(privacyButton);

	std::stringstream title;
	title << appName << " updater";
	SetTitle(title.str());
	//Set to lower right corner
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) - 217;
	int yD = (hS - hD) - 33;
	SetBounds(xD, yD, wD, hD);

	RestoreDeviceObjects();

}

void UpdateCheckDialog::ProcessMessage(GLComponent *src, int message) {

	switch (message) {
	case MSG_BUTTON:
		if (src == allowButton) {
			updater->SetUserUpdatePreference(true);
			updater->RequestUpdateCheck(); //We do an immediate update check: if Windows firewall asks for network connection, the user will know what it is about.
			GLWindow::ProcessMessage(NULL, MSG_CLOSE);
		}
		else if (src == declineButton) {
			updater->SetUserUpdatePreference(false);
			GLWindow::ProcessMessage(NULL, MSG_CLOSE);
		}
		else if (src == laterButton) {
			GLWindow::ProcessMessage(NULL, MSG_CLOSE);
		}
		else if (src == privacyButton) {
			std::string privacyMessage =
R"(When an update check is performed, the server collects visitor statistics
through Google Analytics. The same information is collected as when you visit
any website. To count unique visitors, a client identifier (cookie) is set
on the first update check. This is an anonymous hash that does not contain
anything to identify you. The update check happens when you start the app,
there isn't any network communication later.
)";

			GLMessageBox::Display(privacyMessage.c_str(), "About visitor statistics", GLDLG_OK, GLDLG_ICONINFO);
		}
		break;
	}
	GLWindow::ProcessMessage(src, message);
}

UpdateFoundDialog::UpdateFoundDialog(const std::string & appName, const std::string& appVersionName, AppUpdater* appUpdater)
{
	updater = appUpdater;

	std::stringstream question;
	question << appName << " " << appUpdater->GetLatestUpdateName() << " is available.\n";
	question << "You have " << appName << " " << appVersionName << " (released " __DATE__ ")\n\n"; //Compile-time date
	question << "Would you like to download this version?\nYou don't need to close " << appName << " and it won't overwrite anything.\n\n";
	question << appUpdater->GetCumulativeChangeLog();

	questionLabel = new GLLabel(question.str().c_str());
	questionLabel->SetBounds(5, 5, 150, 60);
	Add(questionLabel);

	int textWidth, textHeight;
	questionLabel->GetTextBounds(&textWidth, &textHeight);

	int wD = Min(800, Max(405, textWidth + 20));  //Dynamic between 405 and 600 pixel width
	int hD = Min(800, Max(100, textHeight + 50)); //Dynamic between 200 and 600 pixel width

	updateButton = new GLButton(0, "Download");
	updateButton->SetBounds(5, hD - 45, 80, 19);
	Add(updateButton);

	laterButton = new GLButton(0, "Ask later");
	laterButton->SetBounds(90, hD - 45, 80, 19);
	Add(laterButton);

	skipButton = new GLButton(0, "Skip version(s)");
	skipButton->SetBounds(175, hD - 45, 95, 19);
	Add(skipButton);

	disableButton = new GLButton(0, "Turn off update check");
	disableButton->SetBounds(275, hD - 45, 120, 19);
	Add(disableButton);

	std::stringstream title;
	title << appName << " updater";
	SetTitle(title.str());
	//Set to lower right corner
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) - 217;
	int yD = (hS - hD) - 33;
	SetBounds(xD, yD, wD, hD);

	RestoreDeviceObjects();

}

void UpdateFoundDialog::ProcessMessage(GLComponent *src, int message) {

	switch (message) {
	case MSG_BUTTON:


		if (src == updateButton) {
			GLMessageBox::Display(updater->InstallLatestUpdate(), "Updater result", { "OK" }, GLDLG_ICONINFO);
			updater->ClearAvailableUpdates();
			GLWindow::ProcessMessage(NULL, MSG_CLOSE);
		}
		else if (src == laterButton) {
			updater->ClearAvailableUpdates();
			GLWindow::ProcessMessage(NULL, MSG_CLOSE);
		}
		else if (src == skipButton) {
			updater->SkipAvailableUpdates();
			updater->ClearAvailableUpdates();
			GLWindow::ProcessMessage(NULL, MSG_CLOSE);
		}
		else if (src == disableButton) {
			updater->ClearAvailableUpdates();
			updater->SetUserUpdatePreference(false);
			GLWindow::ProcessMessage(NULL, MSG_CLOSE);
		}
		break;
	}
	GLWindow::ProcessMessage(src, message);
}