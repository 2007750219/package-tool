#include "PackageUtils.h"

#include "Package.h"

int main()
{
	//D:/Project_2/client2/Game/res  D:/luaTest
	//PackageUtils::packDirectory("D:/luaTest","D:/1.pack");

	Package pack("D:/1.pack",PAT_READ_ONLY);

	bool ret=pack.open();


	if(ret)
	{
		unsigned int fileSize=pack.getFileSize("2.lua");

		char *data=new char[fileSize];

		pack.getFileData("2.lua",data);

		printf("%s",data);

		delete[] data;
	}

	fgetchar();
}

