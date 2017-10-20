#include "recordman.h"
#include <cstring>

namespace	SimpleDataBase
{

bool	RecordManager::createFile(std::string fileName)
{
	if (!fileManager->createFile(fileName)) return false;

	int fileID;
	fileManager->openFile(fileName, fileID);

	int bufIndex;
	BufType b = bufManager->allocPage(fileID, 0, bufIndex, false);
	memset(b, 0, PAGE_SIZE);
	bufManager->markDirty(bufIndex);
	bufManager->writeBack(bufIndex);

	fileManager->closeFile(fileID);
}

bool	RecordManager::removeFile(std::string fileName)
{
	return true;
}

int	openFile(std::string fileName)
{
	int fileID;
	fileManager->openFile(fileName, fileID);
	return fileID;
}

int closeFile(int fileID)
{
	return fileManager->closeFile(fileID);
}

}	// end namespace SimpleDataBase
