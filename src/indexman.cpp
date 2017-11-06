#include "indexman.h"
#include <cstring>

namespace SimpleDataBase
{

bool	IndexManager::createIndex(std::string const& indexName)
{
	if (!fileManager->createFile(indexName.c_str())) return false;

	int fileID;
	fileManager->openFile(indexName.c_str(), fileID);

	int bufIndex;
	BufType b = bufManager->allocPage(fileID, 0, bufIndex, false);
	memset(b, 0, PAGE_SIZE);
	b[0] = 1; b[(PAGE_SIZE >> 2) - 1] = 3;
	bufManager->markDirty(bufIndex);
	bufManager->writeBack(bufIndex);
	b = bufManager->allocPage(fileID, 1, bufIndex, false);
	memset(b, 0, PAGE_SIZE);
	b[0] = 131073;
	bufManager->markDirty(bufIndex);
	bufManager->writeBack(bufIndex);
	b = bufManager->allocPage(fileID, 2, bufIndex, false);
	memset(b, 0, PAGE_SIZE);
	bufManager->markDirty(bufIndex);
	bufManager->writeBack(bufIndex);

	fileManager->closeFile(fileID);
}

}	// end namespace SimpleDataBase
