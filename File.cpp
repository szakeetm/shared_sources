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
#include "File.h"
#include "GLApp/GLTypes.h"
#include <cstring> //strcpy, etc.
#include <filesystem>

#define MAX_WORD_LENGTH 65536 // expected length of the longest line

// FileUtils class

bool FileUtils::Exist(const std::string &fileName) {
    return std::filesystem::exists(fileName);
}

bool FileUtils::Exist(const char *fileName) {
    return std::filesystem::exists(fileName);
}

// FileReader class

FileReader::FileReader(const char *fileName) : fileName{}, readBuffer{} {
    wasLineEnd = false;
    nbLeft = 0;
    buffPos = 0;

    file = fopen(fileName, "r");
    if (!file) {
        char tmp[512];
        sprintf(tmp, "Cannot open file for reading (%s)", fileName);
        throw Error(tmp);
    }

    curLine = 1;
    strcpy(this->fileName, fileName);
    isEof = 0;
    CurrentChar = ' ';
    RefillBuffer();
}

void FileReader::RefillBuffer() {

    if (!isEof) {
        nbLeft = (int) fread(readBuffer, 1, READ_BUFFSIZE, file);
        isEof = (nbLeft == 0);
    }
    buffPos = 0;
}

char FileReader::ReadChar() {

    if (buffPos >= nbLeft)
        RefillBuffer();

    if (!isEof) {
        CurrentChar = readBuffer[buffPos++];
        if (CurrentChar == '\n') {
            curLine++;
            wasLineEnd = true;
        }
    } else {
        CurrentChar = 0;
    }

    return CurrentChar;
}

int FileReader::IsEof() {

    JumpControlChars();
    return isEof;
}

int FileReader::GetCurrentLine() const { return curLine; }

const char *FileReader::GetName() { return fileName; }

FileReader::~FileReader() { fclose(file); }

Error FileReader::MakeError(const char *msg) const {
    static char ret[4096];
    sprintf(ret, "%s (line %d)", msg, curLine);
    return Error(ret);
}

int FileReader::ReadInt() {

    int ret;
    char *w = ReadWord();
    if (sscanf(w, "%d", &ret) <= 0)
        throw Error(MakeError("Wrong integer format"));
    return ret;
}

size_t FileReader::ReadSizeT() {

    size_t ret;
    char *w = ReadWord();
    if (sscanf(w, "%zd", &ret) <= 0) {
        throw Error(MakeError("Wrong integer64 format"));
    }
    return ret;
}

double FileReader::ReadDouble() {

    double ret;
    char *w = ReadWord();
    if (sscanf(w, "%lf", &ret) <= 0) {
        throw Error(MakeError("Wrong double format"));
    }
    return ret;
}

char *FileReader::ReadString() {
    static char retStr[MAX_WORD_LENGTH];

    char *w = ReadWord();
    int len = (int) strlen(w);
    if (w[0] == '"') {
        strncpy(retStr, w + 1, len - 2);
        retStr[len - 2] = 0;
    } else {
        strcpy(retStr, w);
    }

    return retStr;
}

void FileReader::ReadKeyword(const char *keyword) {

    char *w = ReadWord();
    if (strcmp(w, keyword) != 0) {
        char msg[200];
        sprintf(msg,
                "Unexpected keyword in FileReader::ReadkeyWord()\n\"%s\" expected, "
                "\"%s\" found.",
                keyword, w);
        throw Error(MakeError(msg));
    }
}

void FileReader::SeekStart() {
    fseek(file, 0L, SEEK_SET);
    isEof = 0;
    curLine = 1;
    RefillBuffer();
    CurrentChar = ' ';
}

void FileReader::JumpSection(const char *end) {

    char *w = ReadWord();
    while (strcmp(w, end) != 0)
        w = ReadWord();
}

bool FileReader::SeekFor(const char *keyword) {
    char *w;
    bool notFound;
    do {
        w = ReadLine();
        notFound = ((strcmp(w, keyword) != 0) && (!isEof));
    } while (notFound);
    return isEof == 0;
}

bool FileReader::SeekForChar(const char *c) {
    char w;
    bool notFound;
    do {
        w = this->ReadChar();
        notFound = (w != (*c)) && (!isEof);
    } while (notFound);
    return isEof == 0;
}

char *FileReader::ReadLine() {

    static char retWord[MAX_WORD_LENGTH];
    int len = 0;

    /* Jump space and comments */

    JumpControlChars();

    while (CurrentChar >= 32 && len < MAX_WORD_LENGTH) {
        retWord[len] = CurrentChar;
        len++;
        ReadChar();
    }

    if (len >= MAX_WORD_LENGTH) {
        throw MakeError("Line too long");
    }

    retWord[len] = '\0';
    return retWord;
}

void FileReader::JumpComment() {
    JumpControlChars();
    if (CurrentChar == '{') {
        while (!isEof && CurrentChar != '}')
            ReadChar();
        ReadChar();
    }
}

void FileReader::JumpControlChars() {

    // Jump spaces and control characters
    while (!isEof && CurrentChar <= 32)
        ReadChar();
}

int FileReader::IsEol() const {
    return (CurrentChar == '\n' || CurrentChar == '\r');
}

char *FileReader::ReadWord() {

    static char retWord[MAX_WORD_LENGTH];
    int len = 0;

    /* Jump space and comments */

    JumpControlChars();

    /* Treat special character */
    if (CurrentChar == ':' || CurrentChar == '{' || CurrentChar == '}' ||
        CurrentChar == ',') {
        retWord[0] = CurrentChar;
        retWord[1] = '\0';
        ReadChar();
        return retWord;
    }

    /* Treat string */
    if (CurrentChar == '"') {
        retWord[len] = CurrentChar;
        len++;
        ReadChar();
        while (CurrentChar != '"' && CurrentChar != 0 && CurrentChar != '\n' &&
               len < MAX_WORD_LENGTH) {
            retWord[len] = CurrentChar;
            len++;
            ReadChar();
        }
        if (len >= MAX_WORD_LENGTH)
            throw MakeError("String too long");
        if (CurrentChar == 0 || CurrentChar == '\n')
            throw MakeError("String not ended");
        retWord[len] = CurrentChar;
        len++;
        ReadChar();
        retWord[len] = '\0';
        return retWord;
    }

    /* Treat other word */
    while (CurrentChar > 32 && CurrentChar != ':' && CurrentChar != '{' &&
           CurrentChar != '}' && CurrentChar != ',' && len < MAX_WORD_LENGTH) {
        retWord[len] = CurrentChar;
        len++;
        ReadChar();
    }

    if (len >= MAX_WORD_LENGTH) {
        throw MakeError("String too long");
    }

    retWord[len] = '\0';
    return retWord;
}

// FileWriter class

FileWriter::FileWriter(const char *fileName) : fileName{} {

    file = fopen(fileName, "w");
    if (!file) {
        char tmp[256];
        sprintf(tmp, "Cannot open file for writing %s", fileName);
        throw Error(tmp);
    }
    strcpy(this->fileName, fileName);
}

char *FileWriter::GetName() { return fileName; }

FileWriter::~FileWriter() { fclose(file); }

void FileWriter::Write(const int &v, const char *sep) {
    if (!fprintf(file, "%d", v))
        throw Error("Error while writing to file");
    if (sep)
        fprintf(file, "%s", sep);
}

void FileWriter::Write(const size_t &v, const char *sep) {
    //#ifdef _WIN32
    if (!fprintf(file, "%zd", v))
        throw Error("Error while writing to file");
    //#else
    //	throw Error("FileWriter::Write() not implemented"); //commented out:
    //fprintf works just as well on Linux #endif
    if (sep)
        fprintf(file, "%s", sep);
}

void FileWriter::Write(const double &v, const char *sep) {

    // if(v>=0.0) fprintf(file," ");
    fprintf(file, " ");
    if (!fprintf(file, "%.14E", v))
        throw Error("Error while writing to file");
    if (sep)
        fprintf(file, "%s", sep);
}

void FileWriter::Write(const std::string &str) { Write(str.c_str()); }

void FileWriter::Write(const char *s) {
    if (*s == 0)
        return; // null expression: don't do anything (for example formulas without
    // name)
    if (!fprintf(file, "%s", s))
        throw Error("Error while writing to file");
}

std::string FileUtils::GetFilename(const std::string &str) {
    /*
    size_t found = str.find_last_of("/\\");
    if (found == std::string::npos)
        return str; // not found
    return str.substr(found + 1);
    */
    std::filesystem::path path = str; //Convert to path object
    return path.filename().u8string();
}

std::string FileUtils::StripExtension(const std::string &str) {

    size_t lastdot = str.find_last_of('.');
    if (lastdot == std::string::npos)
        return str;
    return str.substr(0, lastdot);

}

std::string FileUtils::GetPath(const std::string &str) {

    size_t found = str.find_last_of("/\\");
    if (found == std::string::npos)
        return ""; // not found, return empty string
    else
        return str.substr(0, found + 1);

}

std::string FileUtils::GetExtension(const std::string &str) {
    //Returns file extension (without starting dot) or empty string if none
    /*
    //Old code, gives wrong extension on "C:\folder.name\file"
    size_t found = str.find_last_of('.');
    if (found == std::string::npos)
        return ""; // not found
    else
        return str.substr(found + 1);
     */
    std::filesystem::path path = str; //Convert to path object
    std::string ext = path.extension().u8string();
    if (ext.length()>0 && ext.at(0)=='.') ext.erase(0,1); //Strip starting dot
    return ext;
}

std::string FileUtils::get_working_path() {
    return std::filesystem::current_path().u8string();
}

void FileUtils::CreateDir(const std::string &path) {
    if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }
}
