#include "recordman.h"
#include <cstring>

namespace	SimpleDataBase
{

RecordManager::RecordManager()
{
	MyBitMap::initConst();
	fileManager = new FileManager();
	bufManager = new BufPageManager(fileManager);
}

RecordManager::~RecordManager()
{
	bufManager->close();
	delete bufManager;
	delete fileManager;
}

bool	RecordManager::createFile(std::string fileName)
{
	if (!fileManager->createFile(fileName.c_str())) return false;

	int fileID;
	fileManager->openFile(fileName.c_str(), fileID);

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

int	RecordManager::openFile(std::string fileName)
{
	int fileID;
	fileManager->openFile(fileName.c_str(), fileID);
	return fileID;
}

int RecordManager::closeFile(int fileID)
{
	bufManager->close();
	return fileManager->closeFile(fileID);
}

}	// end namespace SimpleDataBase
