#ifndef _PACKAGE_H
#define  _PACKAGE_H

#include <cstdio>
#include <string>
#include <vector>
#include <map>

#include "PackageDef.h"



class Package
{
public:

	Package(const std::string& packageName,PackageAccessType accessType=PAT_READ_ONLY);

	~Package();

	bool hasFile(const std::string& fileName);

	unsigned int getFileSize(const std::string& fileName);

	void getFileData(const std::string& fileName,char* data);

	void addFile(const std::string& fileName,const char* fileData,unsigned int fileSize);

	void flush();

	bool open();

protected:

	bool readHeader();

	void readFileEntries();

	void build();

	void addFileOriginal(const std::string& fileName,const char* fileData,unsigned int fileSize);

	void addFileAndCompress(const std::string& fileName,const char* fileData,unsigned int fileSize);

	void getFileDataDirectly(const FileEntry& entry,char* data);

	void getFileDataWithUncompress(const FileEntry& entry,char* data);

protected:

	PackageAccessType mAccessType;

	std::string  mPackageName;

	int mPackageSize;

	bool mIsValid;

	FILE* mFileHandle;

	PackageHeader mHeader;

	std::vector<FileEntry> mFileEntryList;

	std::map<std::string,FileEntry> mNameEntryMap;
};


#endif

