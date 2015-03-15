#ifndef _PACKAGEDEF_H
#define _PACKAGEDEF_H

enum PackageAccessType
{
	PAT_CREATE,
	PAT_READ_ONLY,
	PAT_READ_AND_WRITE
};

const unsigned int PACKAGE_DATA_SIGN = 'GEPD';
const unsigned int PACKAGE_VERSION=1;

const unsigned int MAX_NAME_LENGTH=256;
/**
	PackageHeader
	FileData
	FileEntryData
*/

struct PackageHeader
{
	unsigned int sign;  //package sign

	unsigned int version;  //package version
	
	unsigned int headerSize; //package header size

	unsigned int fileDataSize;//file data size

	unsigned int fileEntryDataSize;//file entries size
	
	unsigned int count; //file count
};


struct FileEntry
{
	char fileName[MAX_NAME_LENGTH];  //fileName

	unsigned int id; //identity id,reserved
	
	unsigned int fileDataOffset; //file data offset

	unsigned int compressed;   //whether compressed,1(yes),0(no)
	
	unsigned int originalFileSize; //original file size 

	unsigned int compressedFileSize;	//file size
	
};




#endif