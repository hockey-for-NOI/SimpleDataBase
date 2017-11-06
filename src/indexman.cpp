#include "indexman.h"

namespace SimpleDataBase
{

bool	IndexManager::createIndex(std::string const& indexName)
{
	if (!fileManager->createFile(fileName.c_str())) return false;

	int fileID;
	fileManager->openFile(fileName.c_str(), fileID);

	int bufIndex;
	BufType b = bufManager->allocPage(fileID, 0, bufIndex, false);
	memset(b, 0, PAGE_SIZE);
	b[0] = 1; b[(PAGE_SIZE >> 2) - 1] = 2;
	bufManager->markDirty(bufIndex);
	bufManager->writeBack(bufIndex);
	BufType b = bufManager->allocPage(fileID, 1, bufIndex, false);
	memset(b, 0, PAGE_SIZE);
	b[0] = 1 << 16; b[1] = 2 << 16;
	bufManager->markDirty(bufIndex);
	bufManager->writeBack(bufIndex);
	BufType b = bufManager->allocPage(fileID, 2, bufIndex, false);
	memset(b, 0, PAGE_SIZE);
	bufManager->markDirty(bufIndex);
	bufManager->writeBack(bufIndex);

	fileManager->closeFile(fileID);
}

}	// end namespace SimpleDataBase
