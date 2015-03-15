#ifndef _PACKAGEUTILS_H
#define  _PACKAGEUTILS_H

#include <string>
#include <vector>


class PackageUtils
{
public:
	
	static void getFileNameList(const std::string& directoryName,std::vector<std::string>& fileList);

	static void scanDirectory(const std::string& directoryName,const std::string& prePath,std::vector<std::string>& fileList);

	static void packDirectory(const std::string& directoryName,const std::string packageName);

};





#endif