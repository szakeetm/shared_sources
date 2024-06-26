

#pragma once

#include <cstdio>
#include <string>
#include <vector>
#include <memory>

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
	static bool isBinarySTL(const std::string& filePath); //throws error
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

  std::string MakeError(const std::string& msg) const;
  int GetCurrentLine() const;

  void JumpComment();
  void JumpControlChars();
  static std::unique_ptr<FileReader> ExtractFrom7zAndOpen(const std::string& fileName, const std::string& geomName);
  std::vector<std::vector<double>> ImportCSV_double();
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
  void Write(const size_t v, const char *sep = NULL);
  void Write(const int v, const char *sep=NULL);
  void Write(const double v, const char *sep=NULL);
  void Write(const char *s);
  void Write(const std::string& str);
  void Write(const bool b);
  

private:

  FILE *file;
  char fileName[2048];
  
};

