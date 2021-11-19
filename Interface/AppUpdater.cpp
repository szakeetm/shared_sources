/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#include "AppUpdater.h"
#include "Web.h"
#include <ZipLib/ZipArchive.h>
#include <ZipLib/ZipArchiveEntry.h>
#include <ZipLib/ZipFile.h>
#include "File.h"
//#include <Windows.h>
#include <filesystem>
#include <sstream>
#include "Helper/MathTools.h" //Contains
#include "Helper/StringHelper.h"

#include "GLApp/GLToolkit.h"
#include "GLApp/GLList.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"

#include "GoogleAnalytics.h"

#ifndef _WIN32
#include <unistd.h> //Get user name
#endif

enum class FetchStatus : int
{
    NONE = 0,
    DOWNLOAD_ERROR = 1,
    OTHER_ERROR = 2, OKAY, PARSE_ERROR
};
/**
* \brief Constructor with initialisation for the App Updater
* \param appName hardcoded name of the app (@see versionId.h)
* \param versionId hardcoded version ID of the app (@see versionId.h)
* \param configFile xml file with the updater config (usually in bin folder)
*/
AppUpdater::AppUpdater(const std::string& appName, const int& versionId, const std::string& configFile) {
	applicationName = appName;
	currentVersionId = versionId;
    lastFetchStatus = 0;
	configFileName = configFile;

	if(!std::filesystem::exists(configFileName))
	    MakeDefaultConfig();
    LoadConfig();

}

/**
* \brief Creates a default config from GoogleAnalytics.h info
*/
void AppUpdater::MakeDefaultConfig(){
    xml_document configDoc;
    xml_node rootNode = configDoc.append_child("UpdaterConfigFile"); //XML specifications require a root node

    xml_node serverNode = rootNode.append_child("ServerConfig");
    serverNode.append_child("RemoteFeed").append_attribute("url") = REMOTE_FEED;
    serverNode.append_child("PublicWebsite").append_attribute("url") = PUBLIC_WEBSITE;
    serverNode.child("PublicWebsite").append_attribute("downloadsPage") = DOWNLOAD_PAGE;
    serverNode.append_child("GoogleAnalytics").append_attribute("projectId") = GA_PROJECT_ID;

    xml_node localConfigNode = rootNode.append_child("LocalConfig");
    localConfigNode.append_child("Permission").append_attribute("allowUpdateCheck") = "false";
    localConfigNode.child("Permission").append_attribute("appLaunchedBeforeAsking") = "0";
    localConfigNode.child("Permission").append_attribute("askAfterNbLaunches") = "3";
    localConfigNode.append_child("Branch").append_attribute("name") = BRANCH_NAME BRANCH_OS_SUFFIX;
    localConfigNode.append_child("GoogleAnalytics").append_attribute("cookie") = "";
    localConfigNode.append_child("SkippedVersions");

    configDoc.save_file(configFileName.c_str());
}

/**
* \brief Save a updater config file in xml format
*/
void AppUpdater::SaveConfig() {
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
	for (auto& version : skippedVersionIds) {
		skippedVerNode.append_child("Version").append_attribute("id") = version;
	}

	configDoc.save_file(configFileName.c_str());
}

/**
* \brief Load updater config file in xml format
*/
void AppUpdater::LoadConfig() {
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
	for (auto& version : skippedVerNode.children("Version")) {
		skippedVersionIds.push_back(version.attribute("id").as_int());
	}
}

/**
* \brief Set user selection for automatic update checks
* \param answer if user wants an automatic update check
*/
void AppUpdater::SetUserUpdatePreference(bool answer) {
	allowUpdateCheck = answer;
	appLaunchedWithoutAsking = -1; //Don't ask again
	SaveConfig();
}

/**
* \brief If a new update shall be marked as skipped, skip and save info
*/
void AppUpdater::SkipAvailableUpdates() {
	SkipVersions(availableUpdates);
	SaveConfig();
}

/**
* \brief Downloads and installs latest update
* \param logWindow window for update process
*/
void AppUpdater::InstallLatestUpdate(UpdateLogWindow* logWindow) {
	UpdateManifest latestUpdate = GetLatest(availableUpdates);
	std::thread t = std::thread(&AppUpdater::DownloadInstallUpdate, this, latestUpdate, logWindow);
	t.detach();
	//DownloadInstallUpdate(GetLatest(availableUpdates),logWindow);
}

/**
* \brief Mark updates as skipped
* \param updates vector containing update IDs to skip
*/
void AppUpdater::SkipVersions(const std::vector<UpdateManifest>& updates) {
	for (auto& update : updates) {
		if (!Contains(skippedVersionIds, update.versionId)) {
			skippedVersionIds.push_back(update.versionId);
		}
	}
}

/**
* \brief Get permission to ask user if he wants to update
* \return if or when user should be asked to update
*/
int AppUpdater::RequestUpdateCheck() {
	if (appLaunchedWithoutAsking == -1) {
		if (allowUpdateCheck) updateThread = std::thread(&AppUpdater::PerformUpdateCheck, (AppUpdater*)this, false); //Launch parallel update-checking thread
		return ANSWER_ALREADYDECIDED;
	}
	else if (appLaunchedWithoutAsking >= askAfterNbLaunches) { //Third time launching app, time to ask if we can check for updates
		return ANSWER_ASKNOW;
	}
	else {
		return ANSWER_DONTASKYET;
	}
}

/**
* \brief Get permission to ask user if he wants to update
* \return if or when user should be asked to update
*/
void AppUpdater::PerformImmediateCheck() {
    PerformUpdateCheck(true);
}

/**
* \brief Check for updates
*/
void AppUpdater::PerformUpdateCheck(bool forceCheck) {
	//Update checker
	if (allowUpdateCheck || forceCheck) { //One extra safeguard to ensure that we (still) have the permission
		//std::string url = "https://molflow.web.cern.ch/sites/molflow.web.cern.ch/files/autoupdate.xml"; //Update feed

		std::string resultCategory;
		std::stringstream resultDetail;
		std::string os = GLToolkit::GetOSName();

		auto[downloadResult, body] = DownloadString(feedUrl);
		//Handle errors
		if (downloadResult == CURLE_OK) {

			pugi::xml_document updateDoc;
			pugi::xml_parse_result parseResult = updateDoc.load_string(body.c_str());
			//Parse document and handle errors

			if (parseResult.status == status_ok) { //parsed successfully
				availableUpdates = DetermineAvailableUpdates(updateDoc, currentVersionId);
                auto availableUpdates_old = DetermineAvailableUpdatesOldScheme(updateDoc, currentVersionId, branchName);
                availableUpdates.insert(availableUpdates.end(), availableUpdates_old.begin(), availableUpdates_old.end());
                resultCategory = "updateCheck";
				resultDetail << "updateCheck_" << applicationName << "_" << currentVersionId;
                lastFetchStatus = (int)FetchStatus::OKAY;
            }
			else { //parse error
				resultCategory = "parseError";
				resultDetail << "parseError_" << parseResult.status << "_" << applicationName << "_" << currentVersionId;
                lastFetchStatus = (int)FetchStatus::PARSE_ERROR;
            }
		}
		else { //download error
			resultCategory = "stringDownloadError";
			resultDetail << "stringDownloadError_" << downloadResult << "_" << applicationName << "_" << currentVersionId;
		    lastFetchStatus = (int)FetchStatus::DOWNLOAD_ERROR;
		}
		//Send result for analytics

		if (Contains({ "","not_set","default" }, userId)) {
			//First update check: generate random install identifier, like a browser cookie (4 alphanumerical characters)
			//It is generated based on the computer's network name and the logged in user name
			//FOR USER PRIVACY, THESE ARE NOT SENT TO GOOGLE ANALYTICS, JUST AN ANONYMOUS HASH
			//Should get the same hash even in case of subsequent installs

			GenerateUserId();
			SaveConfig();
		}

		std::stringstream payload;
		payload << "v=1&t=event&tid=" << googleAnalyticsTrackingId << "&cid=" << userId << "&ec=" << resultCategory << "&ea=" << resultDetail.str();
		payload << "&an=" << applicationName << "&aid=" << branchName << "&av=" << currentVersionId << "_" << os;
		payload << "&el=" << os << "%20" << applicationName << "%20" << currentVersionId << "%20" << branchName;
		SendHTTPPostRequest("http://www.google-analytics.com/collect", payload.str()); //Sends random app and version id for analytics. Also sends install id to count number of users
	}
}

/**
* \brief Retrieve only the latest update
* \param updates list with updates
* \return latest update
*/
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

/**
* \brief Retrieve list with all newer releases
* \param updateDoc xml doc containing all info about the recent updates
* \param currentVersionId version ID of current version
* \return vector with newer releases
*/
std::vector<UpdateManifest> AppUpdater::DetermineAvailableUpdates(const pugi::xml_node& updateDoc, const int& currentVersionId) {
	std::vector<UpdateManifest> availables;

	xml_node rootNode = updateDoc.child("UpdateFeed");
	for (auto& branchNode : rootNode.child("Branches").children("Branch")) { //Look for a child with matching branch name
		std::string currentBranchName = branchNode.attribute("name").as_string();
		if (currentBranchName == BRANCH_NAME) {
			for (xml_node updateNode : branchNode.children("UpdateManifest")) {
				int versionId = updateNode.child("Version").attribute("id").as_int();
				if (!Contains(skippedVersionIds, versionId) && versionId > currentVersionId) {
					UpdateManifest newUpdate;
					newUpdate.versionId = versionId;
					newUpdate.name = updateNode.child("Version").attribute("name").as_string();
					newUpdate.date = updateNode.child("Version").attribute("date").as_string();
                    if(std::strcmp(updateNode.child("Version").attribute("enabled").as_string(),"true") != 0)
                        continue;

					newUpdate.changeLog = updateNode.child("ChangeLog").child_value("Global");
                    //TODO: Consider OS dependent changelogs

                    std::string os_fullname;
                    std::string os_prefix;
                    bool validOS = false;
                    for (xml_node osNode : updateNode.child("ValidForOS").children("OS")) {
                        if(std::strcmp(osNode.attribute("id").as_string() , OS_ID) == 0){
                            // found
                            validOS = true;
                            os_fullname = osNode.attribute("name").as_string();
                            os_prefix = osNode.attribute("prefix").as_string();
                            break;
                        }
                    }
                    if(!validOS)
                        continue; // get next Manifest

                    std::string namedRelease = os_prefix + "_" + newUpdate.name;
					newUpdate.zipUrl = updateNode.child("Content").attribute("zipUrlPathPrefix").as_string();
                    newUpdate.zipName = updateNode.child("Content").attribute("zipName").as_string();
                    if(newUpdate.zipName.empty()){
                        newUpdate.zipName = namedRelease + ".zip";
                    }
                    newUpdate.zipUrl = newUpdate.zipUrl + "/" + namedRelease + "/" + newUpdate.zipName;
                    newUpdate.folderName = updateNode.child("Content").attribute("zipFolder").as_string();
                    if(newUpdate.folderName.empty()){
                        newUpdate.folderName = namedRelease;
                    }

                    // TODO: Add os dependent files
					for (xml_node fileNode : updateNode.child("FilesToCopy").child("Global").children("File")) {
						newUpdate.filesToCopy.emplace_back(fileNode.attribute("name").as_string());
					}
					availables.push_back(newUpdate);
				}
			}
		}
	}
	return availables;
}

// Function to retrieve updates with old XML scheme
std::vector<UpdateManifest> AppUpdater::DetermineAvailableUpdatesOldScheme(const pugi::xml_node& updateDoc, const int& currentVersionId, const std::string& branchName) {
    std::vector<UpdateManifest> availables;

    xml_node rootNode = updateDoc.child("UpdateFeed");
    for (auto& branchNode : rootNode.child("Branches").children("Branch")) { //Look for a child with matching branch name
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
                        newUpdate.filesToCopy.emplace_back(fileNode.attribute("name").as_string());
                    }
                    availables.push_back(newUpdate);
                }
            }
        }
    }
    return availables;
}

/**
* \brief Retrieve string containing a changelog of all newer updates
* \param updates list of updates
* \return cumulative changelog as a string
*/
std::string AppUpdater::GetCumulativeChangeLog(const std::vector<UpdateManifest>& updates) {
	//No sorting: for a nice cumulative changelog, updates should be in chronological order (newest first)
	std::stringstream cumulativeChangeLog;
	for (const auto& update : updates) {
		cumulativeChangeLog << "Changes in version " << update.name << " (released " << update.date << "):\n" << update.changeLog << "\n";
	}
	return cumulativeChangeLog.str();
}

/**
* \brief Retrieve string containing a changelog of all newer updates
* \param updates list of updates
* \return cumulative changelog as a string
*/
std::string AppUpdater::GetLatestChangeLog(const std::vector<UpdateManifest>& updates) {
    if(updates.empty())
        return "";
    //No sorting: for a nice cumulative changelog, updates should be in chronological order (newest first)
    std::stringstream latestChangeLog;
    auto update = GetLatest(updates);
    latestChangeLog << "Changes in version " << update.name << " (released " << update.date << "):\n" << update.changeLog << "\n";
    return latestChangeLog.str();
}

/**
* \brief Generate a user id based on hashed computer name and user name
*/
void AppUpdater::GenerateUserId() {
	char computerName[1024];
	char userName[1024];
#ifdef _WIN32
	DWORD size = 1024;
	GetComputerName(computerName, &size);
	GetUserName(userName, &size);
#else
	size_t size = 1024;
	gethostname(computerName, size);
	getlogin_r(userName, size);
#endif

	std::string id = computerName;
	id += "/";
	id += userName;

	//Create hash code from computer name
	//Hash algorithm source: http://www.cse.yorku.ca/~oz/hash.html
	size_t hashCode = 5381;
	int c;
	size_t index = 0;
	while ((c = id.c_str()[index++])) {
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
		size_t remainder = hashCode - dividend * alphaNum.length();
		hashCode = dividend;
		userId = alphaNum[remainder] + userId;
	}
}

/**
* \brief Starts download and installation of the update
* \param update update which should be downloaded
* \param logWindow window for the update progress
*/
void AppUpdater::DownloadInstallUpdate(const UpdateManifest& update, UpdateLogWindow *logWindow) {
	//logWindow->Log("[Background update thread started.]");

	std::stringstream payload;
	std::string os = GLToolkit::GetOSName();
	payload << "v=1&t=event&tid=" << googleAnalyticsTrackingId << "&cid=" << userId << "&ec=" << "updateStart" << "&ea=" << "updateStart_" << applicationName << "_" << currentVersionId << "_to_" << update.versionId;
	payload << "&an=" << applicationName << "&aid=" << branchName << "&av=" << currentVersionId << "_" << os;
	payload << "&el=" << os << "%20" << applicationName << "%20" << currentVersionId << "%20" << branchName;
	SendHTTPPostRequest("http://www.google-analytics.com/collect", payload.str()); //Sends random app and version id for analytics. Also sends install id to count number of users
	logWindow->Log("Current directory: " + std::filesystem::current_path().u8string());
	logWindow->Log("Downloading update file...");

	std::string resultCategory;
	std::stringstream resultDetail;
	std::stringstream userResult;

	//Download the zipped new version to parent directory
	std::stringstream zipDest; zipDest << "../" << update.zipName;
	auto [dlResult,httpResponse] = DownloadFile(update.zipUrl, zipDest.str());
	if (dlResult == CURLE_OK && httpResponse == 200) { //Download success
		userResult.str("");
		userResult.clear();
		userResult << "Downloaded " << update.zipUrl << " to " << zipDest.str();
		logWindow->Log(userResult.str());

		{
			//Extract it to LOCAL directory (no way to directly extract to parent)
			size_t numitems;
			std::shared_ptr<ZipArchive> zip;
			try {
				zip = ZipFile::Open(zipDest.str());
				numitems = zip->GetEntriesCount();
				userResult.str("");
				userResult.clear();
				userResult << update.zipName << " opened, it contains " << numitems << " files.";
				logWindow->Log(userResult.str());
			}
			catch (...) {
				resultCategory = "zipItemError";
				resultDetail << "zipItemError_cantOpenZip_" << applicationName << "_" << currentVersionId;
				userResult.str(""); userResult.clear();
				userResult << "Couldn't open " << update.zipName;
				logWindow->Log(userResult.str());
				logWindow->Log("Aborting update process.");
				return;
			}

			for (int zi = 0; zi < numitems; zi++)
			{
				auto name = zip->GetEntry(zi)->GetName(); //Filename only, empty for folders
				auto fullName = zip->GetEntry(zi)->GetFullName(); //With path
				//Debug
				//logWindow->Log("name: \"" + name + "\" fullName: \"" + fullName + "\"");
				//End debug
				if (name.empty() /*&& endsWith(fullName, "/")*/) {
					std::string dirName;
					if (endsWith(fullName, "/")) {
						dirName = fullName.substr(0, fullName.size() - 1);
					}
					else
					{
						dirName = fullName;
					}
					//Debug
					//logWindow->Log("Creating directory " + dirName);
					//End debug
					try {
						FileUtils::CreateDir(dirName);
					}
					catch (std::filesystem::filesystem_error err) {
						resultCategory = "zipExtractFolderCreateError";
						resultDetail << "zipExtractFolderCreateError_" << space2underscore(err.what()) << "_item_" << zi << "_name_" << space2underscore(name) << "_" << applicationName << "_" << currentVersionId;
						userResult.str(""); userResult.clear();
						userResult << "Item #" << (zi + 1) << ": couldn't create directory " << dirName << " Maybe it already exists in your app folder (from a previous update),";
						logWindow->Log(userResult.str());
						logWindow->Log("or you don't have permission to write in the destination.");
						logWindow->Log("Aborting update process.");
						return;
					}
					continue;
				}
				auto dest = fullName;
				/*
				#ifdef _WIN32
							std::replace(dest.begin(),dest.end(), '/', '\\');
				#endif
				*/
				try {
					//Debug
					//logWindow->Log("Trying to extract " + fullName + " to " + dest);
					//End debug
					ZipFile::ExtractFile(zipDest.str(), fullName, dest);
				}
				catch (std::runtime_error err) {
					resultCategory = "zipExtractError";
					resultDetail << "zipExtractError_" << space2underscore(err.what()) << "_item_" << zi << "_name_" << name << "_" << applicationName << "_" << currentVersionId;
					userResult.str(""); userResult.clear();
					userResult << "Couldn't extract item #" << (zi + 1) << " (" << fullName << ") of " << update.zipName << " to " << dest << ". Maybe it already exists in your app folder (from a previous update),";
					logWindow->Log(userResult.str());
					logWindow->Log("or you don't have permission to write in the destination.");
					logWindow->Log("Aborting update process.");
					return;
				}
			}

			userResult.str(""); userResult.clear();
			userResult << "All files extracted.";
			logWindow->Log(userResult.str());
		} //Zip goes out of scope, handle released

		//ZIP file not required anymore
		if (remove(zipDest.str().c_str()) != 0) {
			resultCategory = "zipDeleteError";
			resultDetail << "zipDeleteError_" << applicationName << "_" << currentVersionId;
			userResult.str(""); userResult.clear();
			userResult << "Couldn't delete " << update.zipName;
			logWindow->Log(userResult.str());
			logWindow->Log("Aborting update process.");
			return;
		}
		else {
			userResult.str(""); userResult.clear();
			userResult << update.zipName << " deleted.";
			logWindow->Log(userResult.str());
			//Move extracted dir to parent dir
			std::stringstream folderDest; folderDest << "../" << update.folderName;
			if (rename(update.folderName.c_str(), folderDest.str().c_str()) != 0) {
				resultCategory = "folderMoveError";
				resultDetail << "folderMoveError_" << applicationName << "_" << currentVersionId;
				userResult.str(""); userResult.clear();
				userResult << "Couldn't move " << update.folderName << " to " << folderDest.str() << "  The folder already exists or you don't have permission to write there.";
				logWindow->Log(userResult.str());
				logWindow->Log("Aborting update process.");
				return;
			}
			else {
				userResult.str(""); userResult.clear();
				userResult << "Moved the extracted folder " << update.folderName << " to " << folderDest.str();
				logWindow->Log(userResult.str());
				//Copy current config file to new version's dir
				for (auto& copyFile : update.filesToCopy) {
					std::stringstream configDest; configDest << folderDest.str() << "/" << copyFile;
					try {
						std::filesystem::copy(copyFile, configDest.str(), std::filesystem::copy_options::overwrite_existing);
					}
					catch (std::filesystem::filesystem_error err) {
						resultCategory = "fileCopyWarning";
						resultDetail << "fileCopyWarning_" << space2underscore(copyFile) << "_" << applicationName << "_" << currentVersionId;
						userResult.str(""); userResult.clear();
						userResult << "Couldn't copy " << copyFile << " to " << configDest.str() << "  File skipped.";
						logWindow->Log(userResult.str());

						std::stringstream payload2;
						payload2 << "v=1&t=event&tid=" << googleAnalyticsTrackingId << "&cid=" << userId << "&ec=" << resultCategory << "&ea=" << resultDetail.str();
						payload2 << "&an=" << applicationName << "&aid=" << branchName << "&av=" << currentVersionId << "_" << os;
						payload2 << "&el=" << os << "%20" << applicationName << "%20" << currentVersionId << "%20" << branchName;
						SendHTTPPostRequest("http://www.google-analytics.com/collect", payload2.str()); //Sends random app and version id for analytics. Also sends install id to count number of users
						return;

					}
					userResult.str(""); userResult.clear();
					userResult << "Copied " << copyFile << " to " << configDest.str();
					logWindow->Log(userResult.str());
				}
				resultCategory = "updateSuccess";
				resultDetail << "updateSuccess_" << applicationName << "_" << currentVersionId << "_to_" << update.versionId;
				logWindow->Log("Update successful.");
				userResult.str(""); userResult.clear();
				userResult << "If you wish, you can now close this version and launch the new one in the adjacent " << folderDest.str() << " folder.";
				logWindow->Log(userResult.str());
			}
		}
	}
	else {
		resultCategory = "zipDownloadError";
		resultDetail << "zipDownloadError_curl" << dlResult << "_http" << httpResponse << "_" << applicationName << "_" << currentVersionId;
		userResult.str(""); userResult.clear();
		if (dlResult == CURLE_WRITE_ERROR) {
			userResult << "Couldn't write local file " << zipDest.str() << "\nCheck if you have access to write there.";
		}
		else {
			userResult << "Couldn't download " << update.zipUrl << " to " << zipDest.str() << "\nNo network connection or the file doesn't exist on the server.";
			userResult << "\nHttp response code: " << httpResponse;
		}
		logWindow->Log(userResult.str());
		logWindow->Log("Aborting update process.");
	}

	std::stringstream payload3;
	payload3 << "v=1&t=event&tid=" << googleAnalyticsTrackingId << "&cid=" << userId << "&ec=" << resultCategory << "&ea=" << resultDetail.str();
	payload3 << "&an=" << applicationName << "&aid=" << branchName << "&av=" << currentVersionId << "_" << os;
	payload3 << "&el=" << os << "%20" << applicationName << "%20" << currentVersionId << "%20" << branchName;
	SendHTTPPostRequest("http://www.google-analytics.com/collect", payload3.str()); //Sends random app and version id for analytics. Also sends install id to count number of users

	//logWindow->Log("[Background update thread closed.]");
}

/**
* \brief Increase counter of how many  times the application got started
*/
void AppUpdater::IncreaseSessionCount() {
	if (!(appLaunchedWithoutAsking == -1)) {
		appLaunchedWithoutAsking++;
		SaveConfig();
	}
}

/**
* \brief Check if an update is available
* \return true if update available
*/
bool AppUpdater::IsUpdateAvailable() {
	return (!availableUpdates.empty());
}

/**
* \brief Check if an update check is allowed
* \return true if update check is allowed
*/
bool AppUpdater::IsUpdateCheckAllowed() const {
	return allowUpdateCheck;
}

/**
* \brief Clear vector of available updates
*/
void AppUpdater::ClearAvailableUpdates() {
	availableUpdates.clear();
}

/**
* \brief Return a string containing the name of the latest update
* \return string of latest update
*/
std::string AppUpdater::GetLatestUpdateName() {
	std::stringstream name;
	UpdateManifest latestUpdate = GetLatest(availableUpdates);
	name << latestUpdate.name << " (released " << latestUpdate.date << ")";
	return name.str();
}

/**
* \brief Retrieve string containing a changelog of all newer updates
* \return cumulative changelog as a string
*/
std::string AppUpdater::GetCumulativeChangeLog() {
	return GetCumulativeChangeLog(availableUpdates);
}

/**
* \brief Retrieve string containing a changelog of the latest update
* \return latest changelog as a string
*/
std::string AppUpdater::GetLatestChangeLog() {
    return GetLatestChangeLog(availableUpdates);
}

/**
* \brief Constructor with initialisation for the update check dialog
* \param appName name of the application
* \param appUpdater App Updater handle
*/
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

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
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

/**
* \brief Constructor with initialisation for the dialog window if an update has been found
* \param appName name of the application
* \param appVersionName string for the app version
* \param appUpdater App Updater handle
* \param logWindow window for update progress handle
*/
ManualUpdateCheckDialog::ManualUpdateCheckDialog(const std::string & appName, const std::string& appVersionName, AppUpdater* appUpdater, UpdateLogWindow* logWindow, UpdateFoundDialog* foundWindow) {
	this->appName = appName;
    this->appVersionName = appVersionName;
    updater = appUpdater;
	logWnd = logWindow;
    foundWnd = foundWindow;

    std::ostringstream aboutText;
    aboutText << "You have " << appName << " " << appVersionName << " (released " __DATE__ ")\n\n"; //Compile-time date

    questionLabel = new GLLabel(aboutText.str().c_str());
	questionLabel->SetBounds(5, 5, 150, 60);
	Add(questionLabel);

	int textWidth, textHeight;
	questionLabel->GetTextBounds(&textWidth, &textHeight);

    int wD = Min(800, Max(405, textWidth + 20));  //Dynamic between 405 and 600 pixel width
    int hD = Min(800, Max(100, textHeight + 50)); //Dynamic between 200 and 600 pixel width

    updateButton = new GLButton(0, "Download");
    updateButton->SetBounds(5, hD - 45, 80, 19);
    updateButton->SetEnabled(false);
    Add(updateButton);

    cancelButton = new GLButton(0, "Cancel");
    cancelButton->SetBounds(90, hD - 45, 80, 19);
    cancelButton->SetEnabled(true);
    Add(cancelButton);

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

/**
* \brief Performs a new update check and refreshes window content
*/
void ManualUpdateCheckDialog::Refresh() {
    bool updateAvailable = false;
    std::ostringstream aboutText;
    aboutText << "You have " << appName << " " << appVersionName << " (released " __DATE__ ")\n\n"; //Compile-time date

    //Check if app updater has found updates
    bool updateError = true;
    if (updater && logWnd) {
        if (updater->IsUpdateAvailable()) {
            updateAvailable = true;
        }
        else { // no updates found
            // Try again
            updater->PerformImmediateCheck();
            if (updater->IsUpdateAvailable())
                updateAvailable = true;
            if(updater->GetStatus() != (int)FetchStatus::OKAY){
                updateError = true;
            }
            else {
                aboutText << "This is already the latest version!" << std::endl;
                aboutText << updater->GetLatestChangeLog();
            }
        }
    }


    if(updateAvailable){
        aboutText.clear();
        aboutText << appName << " " << updater->GetLatestUpdateName() << " is available.\n";
        //aboutText << "You have " << appName << " " << appVersionName << " (released " __DATE__ ")\n\n"; //Compile-time date
        aboutText << "Would you like to download this version?\nYou don't need to close " << appName << " and it won't overwrite anything.\n\n";
        aboutText << updater->GetLatestChangeLog();
        updateError = false;
    }
    if(updateError){ // app updater error
        aboutText << "Could not check for updates!" << std::endl;
        aboutText << "Please try again after restarting the application or check for a new update at" << std::endl;
        aboutText << "    https://molflow.web.cern.ch/" << std::endl;
    }

    questionLabel->SetText(aboutText.str().c_str());

    int textWidth, textHeight;
    questionLabel->GetTextBounds(&textWidth, &textHeight);

    int wD = Min(800, Max(405, textWidth + 20));  //Dynamic between 405 and 600 pixel width
    int hD = Min(800, Max(100, textHeight + 50)); //Dynamic between 200 and 600 pixel width

    updateButton->SetBounds(5, hD - 45, 80, 19);
    updateButton->SetEnabled(updateAvailable);

    cancelButton->SetBounds(90, hD - 45, 80, 19);
    updateButton->SetEnabled(true);

    //Set to lower right corner
    int wS, hS;
    GLToolkit::GetScreenSize(&wS, &hS);
    int xD = (wS - wD) - 217;
    int yD = (hS - hD) - 33;
    SetBounds(xD, yD, wD, hD);

    RestoreDeviceObjects();
}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void ManualUpdateCheckDialog::ProcessMessage(GLComponent *src, int message) {
    switch (message) {
        case MSG_BUTTON:
            if (src == updateButton) {
                logWnd->ClearLog();
                logWnd->SetVisible(true);
                updater->InstallLatestUpdate(logWnd);
                updater->ClearAvailableUpdates();
                GLWindow::ProcessMessage(NULL, MSG_CLOSE);
            }
            /*else if (src == laterButton) {
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
            }*/
            break;
    }
    GLWindow::ProcessMessage(src, message);
}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void UpdateFoundDialog::ProcessMessage(GLComponent *src, int message) {

	switch (message) {
	case MSG_BUTTON:


		if (src == updateButton) {
			logWnd->ClearLog();
			logWnd->SetVisible(true);
			updater->InstallLatestUpdate(logWnd);
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

/**
* \brief Constructor with initialisation for the dialog window if an update has been found
* \param appName name of the application
* \param appVersionName string for the app version
* \param appUpdater App Updater handle
* \param logWindow window for update progress handle
*/
UpdateFoundDialog::UpdateFoundDialog(const std::string & appName, const std::string& appVersionName, AppUpdater* appUpdater, UpdateLogWindow* logWindow) {
    updater = appUpdater;
    logWnd = logWindow;

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

/**
* \brief Constructor with initialisation for the update log window
* \param app application handle
*/
UpdateLogWindow::UpdateLogWindow(Interface *app) {
	isLocked = true;

	mApp = app;

	int wD = 400;
	int hD = 250;

	logList = new GLList(0);

	logList->SetColumnLabelVisible(true);
	logList->SetSize(1, 1);
	
	//logList->SetColumnWidthForAll(600);
	Add(logList);

	okButton = new GLButton(0, "Dismiss");
	
	Add(okButton);

	copyButton = new GLButton(0, "Copy to clipboard");
	
	Add(copyButton);

	SetTitle("Update log");
	//Set to lower right corner
	//int wS, hS;
	//GLToolkit::GetScreenSize(&wS, &hS);
	//int xD = (wS - wD) - 217;
	int xD = 10;
	int yD = 50;
	SetBounds(xD, yD, wD, hD);
	SetResizable(true);

	RestoreDeviceObjects();
	isLocked = false;
}

/**
* \brief Clears the log
*/
void UpdateLogWindow::ClearLog() {
	lines.clear();
	if (!isLocked) {
		isLocked = true;
		RebuildList();
		mApp->wereEvents = true;
		isLocked = false;
	}
	else {
		DEBUG_BREAK;
	}
}

/**
* \brief Appends line to the log
* \param line line that gets appended to the log
*/
void UpdateLogWindow::Log(const std::string & text) {
	auto newLines = SplitString(text, '\n');
	lines.insert(lines.end(),newLines.begin(),newLines.end());
	if (!isLocked) {
		isLocked = true;
		RebuildList();
		mApp->wereEvents = true;
		isLocked = false;
	}
	else {
		DEBUG_BREAK;
	}
}

/**
* \brief Rebuilds the log
*/
void UpdateLogWindow::RebuildList() {
	int oldColumnWidth = logList->GetColWidth(0);
	logList->SetSize(1, lines.size(), false, false);
	logList->SetColumnWidth(0, oldColumnWidth); //Restore after SetSize reset it to default
	for (size_t i = 0; i < lines.size(); i++) {
		logList->SetValueAt(0, i, lines[i].c_str());
	}
}

/**
* \brief Sets positions and sizes of the window
* \param x x-coordinate of the element
* \param y y-coordinate of the element
* \param w width of the element
* \param h height of the element
*/
void UpdateLogWindow::SetBounds(int x, int y, int w, int h) {
	logList->SetBounds(5, 5, w - 10, h - 60);
	logList->SetColumnWidth(0, w - 25); //Leave space for vertical scrollbar
	okButton->SetBounds(10, h - 45, 80, 19);
	copyButton->SetBounds(w - 115, h - 45, 100, 19);
	GLWindow::SetBounds(x, y, w, h);
}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void UpdateLogWindow::ProcessMessage(GLComponent *src, int message) {

	switch (message) {
	case MSG_BUTTON:


		if (src == okButton) {
			GLWindow::ProcessMessage(NULL, MSG_CLOSE);
		}
		else if (src == copyButton) {
			std::ostringstream text;
			for (auto& line : lines)
				text << line << "\n";
			GLToolkit::CopyTextToClipboard(text.str());
		}

		break;
	}
	GLWindow::ProcessMessage(src, message);
}
