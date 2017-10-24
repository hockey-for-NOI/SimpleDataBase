#include "recordman.h"
#include <cstring>

namespace	SimpleDataBase
{

RecordManager::RecordManager(
		std::shared_ptr<FileManager> fm,
		std::shared_ptr<BufPageManager> bfm)
{
	MyBitMap::initConst();
	fileManager = fm ? fm : std::make_shared<FileManager>();
	bufManager = bfm ? bfm : std::make_shared<BufPageManager>(fileManager);
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
