

#include "SettingsIO.h"
#include <Helper/ConsoleLogger.h>
#include <Helper/StringHelper.h>
#include <filesystem>
#include "GLApp/GLTypes.h"

// zip
#include <File.h>
#include <ziplib/Source/ZipLib/ZipArchive.h>
#include <ziplib/Source/ZipLib/ZipFile.h>

// Input Output related settings and handy functions for the CLI application
namespace SettingsIO {

    const std::string supportedFileFormats[]{".xml", ".zip", ".syn", ".syn7z"};

    //! prepares work/output directories and unpacks from archive
    void prepareIO(CLIArguments& parsedArgs) {
        if (initDirectories(parsedArgs)) {
            throw Error("Error preparing folders\n");
        }

        if (initFromZip(parsedArgs)) {
            throw Error("Error handling input from zip file\n");
        }
    }

//! Set all directory / output related variables and initialises directories depending on set parameters
    int initDirectories(CLIArguments& parsedArgs) {

        int err = 0;

        // Overwrite excludes outputpath/filename
        if (parsedArgs.overwrite) {
            parsedArgs.outputFile = parsedArgs.inputFile;
            std::string tmpFolder = MFMPI::world_size > 1 ? fmt::format("tmp{}/",MFMPI::world_rank) : "tmp/";
            parsedArgs.workPath = tmpFolder;
        } else if(!parsedArgs.outputPath.empty()){
            parsedArgs.workPath = parsedArgs.outputPath;
        }
        else if (parsedArgs.outputPath.empty() && std::filesystem::path(parsedArgs.outputFile).has_parent_path()) { // Use a default outputpath if unset
            parsedArgs.workPath = std::filesystem::path(parsedArgs.outputFile).parent_path().string();
        } else if (parsedArgs.outputPath.empty()) { // Use a default outputpath if unset
            parsedArgs.outputPath = "Results_" + Util::getTimepointString();
            parsedArgs.workPath = parsedArgs.outputPath;
        }
        else if (std::filesystem::path(parsedArgs.outputFile).has_parent_path()) {
            Log::console_error(
                    "Output path was set to {}, but Output file also contains a parent "
                    "path {}\n"
                    "Output path will be appended!\n",
                    parsedArgs.outputPath,
                    std::filesystem::path(parsedArgs.outputFile).parent_path().string());
        }

        // Use a default outputfile name if unset
        if (parsedArgs.outputFile.empty())
            parsedArgs.outputFile =
                    "out_" +
                    std::filesystem::path(parsedArgs.inputFile).filename().string();

        bool formatIsSupported = false;
        for (const auto &format : supportedFileFormats) {
            if (std::filesystem::path(parsedArgs.outputFile).extension().string() ==
                format) {
                formatIsSupported = true;
            }
        }
        if (!formatIsSupported) {
            Log::console_error("File format is not supported: {}\n",
                               std::filesystem::path(parsedArgs.outputFile).extension().string());
            return 1;
        }

        // Try to create directories
        // First for outputpath, with tmp/ and lastly ./ as fallback plans
        try {
            if (!std::filesystem::exists(parsedArgs.workPath))
                std::filesystem::create_directory(parsedArgs.workPath);
        } catch (const std::exception &) {
            Log::console_error("Couldn't create directory [ {} ], falling back to "
                               "tmp folder for output files\n",
                               parsedArgs.workPath);
            ++err;

            // use fallback dir
            parsedArgs.workPath = MFMPI::world_size > 1 ? fmt::format("tmp{}/",MFMPI::world_rank) : "tmp/";
            try {
                if (!std::filesystem::exists(parsedArgs.workPath))
                    std::filesystem::create_directory(parsedArgs.workPath);
            } catch (const std::exception &) {
                parsedArgs.workPath = "./";
                Log::console_error("Couldn't create fallback directory [ tmp/ ], falling "
                                   "back to binary folder instead for output files\n");
                ++err;
            }
        }

        // Next check if outputfile name has parent path as name
        // Additional directory in outputpath
        if (std::filesystem::path(parsedArgs.outputFile).has_parent_path()) {
            std::string outputFilePath;
            if(parsedArgs.outputPath.empty())
                outputFilePath =
                        std::filesystem::path(parsedArgs.outputFile).parent_path().string();
            else
                outputFilePath =
                        std::filesystem::path(parsedArgs.outputPath)
                                .append(std::filesystem::path(parsedArgs.outputFile)
                                                .parent_path()
                                                .string())
                                .string();
            try {
                if (!std::filesystem::exists(outputFilePath))
                    std::filesystem::create_directory(outputFilePath);
            } catch (const std::exception &) {
                Log::console_error(
                        "Couldn't create parent directory set by output filename [ {} ], "
                        "will only use default output path instead\n",
                        outputFilePath);
                ++err;
            }
        }

        return 0;
    }

    //! Unzip file and set correct variables (e.g. uncompressed work file)
    int initFromZip(CLIArguments& parsedArgs) {
        if (std::filesystem::path(parsedArgs.inputFile).extension() == ".zip") {
            //parsedArgs.isArchive = true;

            // decompress file
            std::string parseFileName;
            Log::console_msg_master(2, "Decompressing {} ...\n", parsedArgs.inputFile);

            ZipArchive::Ptr zip = ZipFile::Open(parsedArgs.inputFile);
            if (zip == nullptr) {
                Log::console_error("Can't open ZIP file\n");
                return 1;
            }
            size_t numItems = zip->GetEntriesCount();
            bool notFoundYet = true;
            for (int i = 0; i < numItems && notFoundYet;
                 i++) { // extract first xml file found in ZIP archive
                auto zipItem = zip->GetEntry(i);
                std::string zipFileName = zipItem->GetName();

                if (std::filesystem::path(zipFileName).extension() ==
                    ".xml") { // if it's an .xml file
                    notFoundYet = false;

                    std::string tmpFolder = MFMPI::world_size > 1 ? fmt::format("tmp{}/",MFMPI::world_rank) : "tmp/";
                    if (parsedArgs.outputPath != tmpFolder)
                        FileUtils::CreateDir(tmpFolder); // If doesn't exist yet

                    parseFileName = tmpFolder + zipFileName;
                    ZipFile::ExtractFile(parsedArgs.inputFile, zipFileName, parseFileName);
                }
            }
            if (parseFileName.empty()) {
                Log::console_error("Zip file does not contain a valid geometry file!\n");
                return 1;
            }
            parsedArgs.workFile = parseFileName;
            //Log::console_msg_master(2, "New input file: {}\n", parsedArgs.workFile); //Will be printed a line later
        } else {
            parsedArgs.workFile = parsedArgs.inputFile;
        }
        return 0;
    }

//! Cleanup tmp files
    void cleanup_files(const std::string& outputPath, const std::string& workPath) {
        // a) tmp folder if it is not our output folder
        if (std::filesystem::path(outputPath)
                    .relative_path()
                    .compare(std::filesystem::path("tmp")) &&
            std::filesystem::path(outputPath)
                    .parent_path()
                    .compare(std::filesystem::path("tmp"))) {
            // CLIArguments::tmpfile_dir
            Log::console_msg_master(2, "Removing tmp directory...");
            std::filesystem::remove_all("tmp");
            Log::console_msg_master(2, "done.\n");
        }

        if(!workPath.empty() && is_empty(std::filesystem::path(workPath))){
            Log::console_msg_master(2, "Removing empty work directory {} ...",workPath);
            std::filesystem::remove_all(workPath);
            Log::console_msg_master(2, "done.\n");
        }
    }
} // namespace SettingsIO