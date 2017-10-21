#include "recordman.h"
#include <cstring>

namespace	SimpleDataBase
{

RecordManager::RecordManager()
{
	MyBitMap::initConst();
	fileManager = new FileManager();
	bufMananger = new BufPageManager(fm);
}

RecordManager::~RecordManager()
{
	bufManager->close();
	delete bufManager;
	delete fileManager;
}

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

int	RecordManager::openFile(std::string fileName)
{
	int fileID;
	fileManager->openFile(fileName, fileID);
	return fileID;
}

int RecordManager::closeFile(int fileID)
{
	bufManager->close();
	return fileManager->closeFile(fileID);
}

ushort	RecordManager::_pageInsert(int pageID, void const* objptr, size_t size)
{
	int bufIndex;
	BufType b = bufMananger->getPage(fileID, pageID, bufIndex);
	ushort &indexnum = b[PAGE_SIZE - 1];
	ushort tail = b[PAGE_SIZE - indexnum - 1];
	if (tail + size + indexnum + 1 < PAGE_SIZE)
	{
		b[PAGE_SIZE - (++indexnum) - 1] = tail + size;
		memcpy(b + tail, objptr, size);
		bufManager->markDirty(bufIndex);
	} else return 65535u;
}

RecordPos	RecordManager::insert(int fileID, void const* objptr, size_t size)
{
	if (size > PAGE_SIZE - 2) return RecordPos(0, 0);
	bool flag = (size + 2 < (PAGE_SIZE >> 1));
	// Try to find a slot in existing pages if it's not so big
	
	int pageID = 0;

	int bufIndex;
	BufType b = bufManager->getPage(fileID, pageID, bufIndex);
	ushort pagenum = b[PAGE_SIZE-1];
	
	while (true)
	{
		if (flag)
		{
			for (ushort byte_index=0; (byte_index << 3) < pagenum; byte_index++)
				for (ushort bit_index=0; (byte_index << 3 | bit_index)<pagenum; bit_index++)
					if (!(b[byte_index] & (1 << bit_index)))
					{
						ushort slotID = _pageInsert(pageID + (byte_index << 3 | bit_index) + 1, objptr, size);
						if (slotID != 65535u)
							return RecordPos(pageID, slotID);
						else
						{
							b = bufManager->getPage(fileID, pageID, bufIndex);
							b[byte_index] |= (1 << bit_index);
							bufManager->markDirty(bufIndex);
						}
					}
		}
		if (pagenum < FILE_STEP) break;
		p.pageID += FILE_STEP;
		b = bufManager->getPage(fileID, pageID, bufIndex);
		pagenum = b[PAGE_SIZE-1];
	}

	// Check and add a flag page
	if (pagenum == FILE_STEP - 1)
	{
		b[PAGE_SIZE-1] = FILE_STEP;
		bufManager->markDirty(bufIndex);
		pageID += FILE_STEP;
		b = bufManager->allocPage(fileID, pageID, bufIndex, false);
		memset(b, 0, PAGE_SIZE);
		pagenum = 0;
	}

	// Add a new slot page
	b[PAGE_SIZE-1] = ++pagenum;
	bufManager->markDirty(bufIndex);
	b = bufManager->allocPage(fileID, pageID + pagenum, bufIndex, false);
	memset(b, 0, PAGE_SIZE);
	ushort slotID = _pageInsert(pageID + pagenum, objptr, size);
	return RecordPos(pageID, slotID);
}

}	// end namespace SimpleDataBase
