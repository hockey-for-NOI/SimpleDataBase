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

ushort	RecordManager::_pageInsert(int fileID, int pageID, void const* objptr, size_t size)
{
	int bufIndex;
	int page_ssize = PAGE_SIZE >> 1;
	ushort* b = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
	ushort &indexnum = b[page_ssize - 1];
	ushort tail = b[page_ssize - indexnum - 1];
	if (tail + size + indexnum + 1 < PAGE_SIZE)
	{
		b[page_ssize - (++indexnum) - 1] = tail + size;
		memcpy(((uchar*)b) + tail, objptr, size);
		bufManager->markDirty(bufIndex);
		return indexnum - 1;
	} else return 65535u;
}

RecordPos	RecordManager::insert(int fileID, void const* objptr, size_t size)
{
	if (size > PAGE_SIZE - 2) return RecordPos(0, 0);
	int page_ssize = PAGE_SIZE >> 1;
	bool flag = (size + 2 < page_ssize);
	// Try to find a slot in existing pages if it's not so big
	
	int pageID = 0;

	int bufIndex;
	ushort* b = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
	ushort pagenum = b[page_ssize - 1];
	
	while (true)
	{
		if (flag)
		{
			for (ushort byte_index=0; (byte_index << 4) < pagenum; byte_index++)
				for (ushort bit_index=0; (!(bit_index & 16)) && (byte_index << 4 | bit_index)<pagenum; bit_index++)
					if (!(b[byte_index] & (1 << bit_index)))
					{
						ushort slotID = _pageInsert(fileID, pageID + (byte_index << 4 | bit_index) + 1, objptr, size);
						if (slotID != 65535u)
							return RecordPos(pageID + (byte_index << 4 | bit_index) + 1, slotID);
						else
						{
							b = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
							b[byte_index] |= (1 << bit_index);
							bufManager->markDirty(bufIndex);
						}
					}
		}
		if (pagenum < FILE_STEP) break;
		pageID += FILE_STEP;
		b = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
		pagenum = b[page_ssize-1];
	}

	// Check and add a flag page
	if (pagenum == FILE_STEP - 1)
	{
		b[page_ssize-1] = FILE_STEP;
		bufManager->markDirty(bufIndex);
		pageID += FILE_STEP;
		b = (ushort*)(bufManager->allocPage(fileID, pageID, bufIndex, false));
		memset(b, 0, PAGE_SIZE);
		pagenum = 0;
	}

	// Add a new slot page
	b[page_ssize-1] = ++pagenum;
	bufManager->markDirty(bufIndex);
	b = (ushort*)(bufManager->allocPage(fileID, pageID + pagenum, bufIndex, false));
	memset(b, 0, PAGE_SIZE);
	ushort slotID = _pageInsert(fileID, pageID + pagenum, objptr, size);
	return RecordPos(pageID + pagenum, slotID);
}

}	// end namespace SimpleDataBase
