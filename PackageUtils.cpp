#include "PackageUtils.h"

#include <io.h>
#include <cstdio>
#include "Package.h"

void PackageUtils::getFileNameList(const std::string& directoryName,std::vector<std::string>& fileList)
{
	PackageUtils::scanDirectory(directoryName,std::string(""),fileList);
}

void PackageUtils::scanDirectory(const std::string& directoryName,const std::string& prePath,std::vector<std::string>& fileList)
{
	_finddata_t file;

	long longf;


	std::string scanDirectoryName=directoryName+"/*.*";


	/**
		long __cdecl _findfirst(const char *, struct _finddata_t *)
	*/
	if((longf = _findfirst(scanDirectoryName.c_str(), &file))==-1l)
	{
		printf("目录%s没有找到!\n",directoryName.c_str());
	}
	else
	{
		/**
			int __cdecl _findnext(long, struct _finddata_t *);
			@return:0(success),-1(failed)
		*/

		std::string tempName;

		while( _findnext( longf, &file ) == 0 )
		{
			tempName = "";
			tempName = file.name;
			if (tempName == "..")
			{
				continue;
			}

			//printf("%s",file.name);

		/*	if(file.attrib == _A_NORMAL)
			{
				printf("  普通文件  ");
			}
			else if(file.attrib == _A_RDONLY)
			{
				printf("  只读文件  ");
			}
			else if(file.attrib == _A_HIDDEN )
			{
				printf("  隐藏文件  ");
			}
			else if(file.attrib == _A_SYSTEM )
			{
				printf("  系统文件  ");
			}
			else if(file.attrib == _A_SUBDIR)
			{
				printf("  子目录  ");

				std::string subDirectoryName=directoryName+"/"+tempName;

				PackageUtils::scanDirectory(subDirectoryName,fileList);
			}
			else 
			{
				printf("  存档文件  ");
			}  */

			if(file.attrib == _A_SUBDIR)
			{
				std::string subDirectoryName=directoryName+"/"+tempName;
				std::string subPrePath;
				
				if(prePath.empty())
				{
					subPrePath=tempName;
				}else
				{
					subPrePath=prePath+"/"+tempName;
				}

				PackageUtils::scanDirectory(subDirectoryName,subPrePath,fileList);

			}else
			{
				if(prePath.empty())
				{
					fileList.push_back(tempName);
				}else
				{
					fileList.push_back(prePath+"/"+tempName);
				}
			}
		}
	}

	_findclose(longf);
}

void PackageUtils::packDirectory(const std::string& directoryName,const std::string packageName)
{
	Package pack(packageName,PAT_CREATE);
	
	bool ret=pack.open();

	if(ret)
	{
		printf("Begin to scan directory %s\n",directoryName.c_str());

	}else
	{
		printf("Open file %s failed!\n",packageName.c_str());
		return;
	}

	std::vector<std::string> fileList;
	getFileNameList(directoryName,fileList);


	std::vector<char> buffer;

	for (int i=0;i<fileList.size();i++)
	{
		std::string fileName=directoryName+"/"+fileList[i];

		FILE* handle=fopen(fileName.c_str(),"rb");

		if(handle==NULL)
		{
			printf("Open file %s failed!\n",fileName.c_str());
			continue;
		}else
		{
			printf("pack file %s\n",fileName.c_str());
		}

		fseek(handle, 0, SEEK_END);

		unsigned int fileSize=ftell(handle);

		if(fileSize>buffer.size())
		{
			buffer.resize(fileSize);
		}

		fseek(handle, 0, SEEK_SET);

		fread(&buffer[0],fileSize,1,handle);

		fclose(handle);

		pack.addFile(fileList[i],&buffer[0],fileSize);
	}

	pack.flush();

	printf("Packing directory %s successful\n",directoryName.c_str());
}
