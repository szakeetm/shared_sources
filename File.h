/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
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

#ifndef FILERWH
#define FILERWH

#include <cstdio>
#include <string>
#include <vector>
#include "GLApp/GLTypes.h"

#define READ_BUFFSIZE 4096

class FileUtils {

public:
	// Utils functions
	static bool Exist(const std::string& fileName);
	static bool Exist(const char* fileName);
	static std::string GetPath(const std::string& str); //Extracts string up to to last "\" (inlcuding "\"). If no path found, returns empty string
	static std::string GetFilename(const std::string& str); //Extracts string after the last "\"
	static std::string StripExtension(const std::string& str);
	static std::string GetExtension(const std::string& str); //Extracts string after the last "."
	//static bool Copy(const std::string& src, const std::string& dst);
	static std::string get_working_path();
	static void CreateDir(const std::string& path);
	static std::string exec(const std::string& command);
	static std::string exec(const char* cmd);

};

class FileReader {

public:
  // Constructor/Destructor
	FileReader(std::string fileName) :FileReader(fileName.c_str()){};
	FileReader(const char *fileName);
	~FileReader();

  const char * GetName();

  // Read function
  int IsEof();
  int IsEol() const;
  char *ReadLine();
  char *ReadString();
  size_t ReadSizeT();
  int ReadInt();
  double ReadDouble();
  void ReadKeyword(const char *keyword);
  bool PeekKeyword(const char *keyword);
  char *ReadWord();
  void JumpSection(const char *end);
  void SeekStart();
  bool SeekFor(const char *keyword);
  bool SeekForInline(const char *keyword);
  bool SeekForChar(const char *c);
  bool wasLineEnd;

  Error MakeError(const char *msg) const;
  int GetCurrentLine() const;

  void JumpComment();
  void JumpControlChars();
  static FileReader* ExtractFrom7zAndOpen(const std::string& fileName, const std::string& geomName);

  std::vector<std::vector<std::string>> ImportCSV_string();
private:

  void RefillBuffer();
  char ReadChar();
  
  
  FILE *file;
  int curLine;
  char fileName[2048];
  char readBuffer[READ_BUFFSIZE];
  char* bufferedKeyword;
  bool peekedKeyword;
  int  nbLeft;
  int  buffPos;
  int  isEof;
  char CurrentChar;
};

class FileWriter {

public:
  // Constructor/Destructor
  FileWriter(std::string fileName) :FileWriter(fileName.c_str()) {};
  FileWriter(const char *fileName);
  ~FileWriter();

  char *GetName();

  // Write function
  void Write(const size_tv, const char *sep = NULL);
  void Write(const intv, const char *sep=NULL);
  void Write(const doublev, const char *sep=NULL);
  void Write(const char *s);
  void Write(const std::string& str);
  

private:

  FILE *file;
  char fileName[2048];
  
};

#endif /* FILERWH */

