#include "Package.h"
#include "zlib/zlib.h"

Package::Package(const std::string& packageName,PackageAccessType accessType):
	mPackageName(packageName),
	mPackageSize(0),
	mIsValid(false),
	mFileHandle(NULL),
	mAccessType(accessType)
{
	
	
}

Package::~Package()
{
	if(mFileHandle)
	{
		fclose(mFileHandle);
	}
}


bool Package::open()
{

	switch (mAccessType)
	{
	case PAT_READ_ONLY:
		mFileHandle=fopen(mPackageName.c_str(),"rb");
		break;
	case PAT_READ_AND_WRITE:
		mFileHandle=fopen(mPackageName.c_str(),"r+b");
		break;
	case PAT_CREATE:
		mFileHandle=fopen(mPackageName.c_str(),"w+b");
		break;
	}
	
	if(mFileHandle==NULL)
	{
		return false;
	}


	if(mAccessType==PAT_CREATE)
	{
		mHeader.sign=PACKAGE_DATA_SIGN;
		mHeader.version=PACKAGE_VERSION;
		mHeader.headerSize=sizeof(PackageHeader);
		mHeader.fileDataSize=0;
		mHeader.fileEntryDataSize=0;
		mHeader.count=0;

		mIsValid=true;
	}else
	{
		fseek(mFileHandle, 0, SEEK_END);
		mPackageSize = ftell(mFileHandle);

		if(mPackageSize<sizeof(PackageHeader))
		{
			mIsValid=false;
			return false;
		}

		if(!readHeader())
		{
			mIsValid=false;
			return false;
		}

		if(mHeader.count>0)
		{
			readFileEntries();
			build();
		}
	}


	
	return true;
}

bool Package::readHeader()
{
	
	fseek(mFileHandle, 0, SEEK_SET);
	
	fread(&mHeader,sizeof(PackageHeader), 1, mFileHandle);

	if(mPackageSize!=mHeader.headerSize+mHeader.fileDataSize+mHeader.fileEntryDataSize)
	{
		mIsValid=false;
		return false;
	}

	if(mHeader.sign!=PACKAGE_DATA_SIGN)
	{
		mIsValid=false;
		return false;
	}

	if(mHeader.version!=PACKAGE_VERSION)
	{
		mIsValid=false;
		return false;
	}

	return true;
	
}

void Package::readFileEntries()
{	
	fseek(mFileHandle, mHeader.headerSize+mHeader.fileDataSize, SEEK_SET);
	
	mFileEntryList.resize(mHeader.count);
	
	fread(&mFileEntryList[0],mHeader.fileEntryDataSize,1,mFileHandle);
}

void Package::build()
{
	for (int i=0;i<mFileEntryList.size();i++)
	{
		mNameEntryMap.insert(std::make_pair(mFileEntryList[i].fileName,mFileEntryList[i]));
	}
}

void Package::addFile(const std::string& fileName,const char* fileData,unsigned int fileSize)
{
	addFileAndCompress(fileName,fileData,fileSize);
}


void Package::addFileOriginal(const std::string& fileName,const char* fileData,unsigned int fileSize)
{

	unsigned int location=mHeader.headerSize+mHeader.fileDataSize;

	//modify package head
	mHeader.count++;
	mHeader.fileDataSize+=fileSize;
	mHeader.fileEntryDataSize+=sizeof(FileEntry);

	//add fileEntry to list
	FileEntry entry;

	memset(entry.fileName,0,sizeof(entry.fileName));
	memcpy(entry.fileName,fileName.c_str(),fileName.length());
	entry.fileName[fileName.length()+1]='\0';
	entry.id=0;
	entry.fileDataOffset=location;

	entry.originalFileSize=fileSize;
	entry.compressedFileSize=0;

	entry.compressed=0;


	mFileEntryList.push_back(entry);


	//write file data
	fseek(mFileHandle, location, SEEK_SET);
	fwrite(fileData,fileSize,1,mFileHandle);
}


void Package::addFileAndCompress(const std::string& fileName,const char* fileData,unsigned int fileSize)
{
	unsigned long compressedSize=(fileSize + 12)*1.001;

	std::vector<char> buffer(compressedSize);  

	compress((Bytef*)&buffer[0],&compressedSize,(const Bytef*)fileData,fileSize);


	unsigned int location=mHeader.headerSize+mHeader.fileDataSize;

	//modify package head
	mHeader.count++;
	mHeader.fileDataSize+=compressedSize;
	mHeader.fileEntryDataSize+=sizeof(FileEntry);

	//add fileEntry to list
	FileEntry entry;

	memset(entry.fileName,0,sizeof(entry.fileName));
	memcpy(entry.fileName,fileName.c_str(),fileName.length());
	entry.fileName[fileName.length()+1]='\0';
	entry.id=0;
	entry.fileDataOffset=location;

	entry.originalFileSize=fileSize;
	entry.compressedFileSize=compressedSize;

	//entry.compressed=0;
	entry.compressed=1;

	mFileEntryList.push_back(entry);


	//write file data
	fseek(mFileHandle, location, SEEK_SET);
	fwrite(&buffer[0],compressedSize,1,mFileHandle);
}

void Package::flush()
{
	//write package head
	fseek(mFileHandle, 0, SEEK_SET);
	fwrite(&mHeader,sizeof(PackageHeader),1,mFileHandle);

	//write FileEntryList
	fseek(mFileHandle, mHeader.headerSize+mHeader.fileDataSize, SEEK_SET);
	fwrite(&mFileEntryList[0],mHeader.count*sizeof(FileEntry),1,mFileHandle);

	fflush(mFileHandle);
}


bool Package::hasFile(const std::string& fileName)
{
	if (mNameEntryMap.count(fileName))
	{
		return true;
	}

	return false;
}


unsigned int Package::getFileSize(const std::string& fileName)
{
	if (mNameEntryMap.count(fileName))
	{
		FileEntry& entry=mNameEntryMap[fileName];

		return entry.originalFileSize;
	}

	return 0;
}


void Package::getFileData(const std::string& fileName,char* data)
{
	if (mNameEntryMap.count(fileName))
	{
		FileEntry& entry=mNameEntryMap[fileName];

		if (entry.compressed==0)
		{
			getFileDataDirectly(entry,data);
		}else
		{
			getFileDataWithUncompress(entry,data);
		}
	}
}


void Package::getFileDataDirectly(const FileEntry& entry,char* data)
{
	fseek(mFileHandle,entry.fileDataOffset,SEEK_SET);

	fread(data,entry.originalFileSize,1,mFileHandle);
}


void Package::getFileDataWithUncompress(const FileEntry& entry,char* data)
{
	fseek(mFileHandle,entry.fileDataOffset,SEEK_SET);

	std::vector<char> buffer(entry.compressedFileSize);

	fread(&buffer[0],entry.compressedFileSize,1,mFileHandle);

	unsigned long originalFileSize=entry.originalFileSize;

	uncompress((Bytef*)data,&originalFileSize,(Bytef*)&buffer[0],entry.compressedFileSize);
}


