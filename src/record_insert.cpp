#include "recordman.h"
#include <cstring>

namespace	SimpleDataBase
{

short	RecordManager::_pageInsert(int fileID, unsigned pageID, void const* objptr)
{
	ushort size; memcpy(&size, objptr, 2);
	int bufIndex;
	uchar* b = (uchar*)(bufManager->getPage(fileID, pageID, bufIndex));
	short* sb = (short*)b;
	short &indexnum = sb[PAGE_SSIZE - 2];
	short &tail = sb[PAGE_SSIZE - 1];
	if (tail + size + (indexnum << 1) + 4 <= PAGE_SIZE)
	{
		sb[PAGE_SSIZE - (++indexnum) - 2] = tail;
		memcpy(b + tail, objptr, size);
		tail += size;
		bufManager->markDirty(bufIndex);
		return indexnum - 1;
	} else return -1;
}

RecordPos	RecordManager::insert(int fileID, void const* objptr)
{
	ushort size; memcpy(&size, objptr, 2);
	if (size > PAGE_SIZE - 6) return RecordPos(0, 0);
	bool flag = (size + 2 < PAGE_SSIZE);
	// Try to find a slot in existing pages if it's not so big
	
	unsigned pageID = 0;

	int bufIndex;
	ushort* b = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
	ushort pagenum = b[PAGE_SSIZE - 1];
	
	while (true)
	{
		if (flag)
		{
			for (ushort byte_index=0; (byte_index << 4) < pagenum; byte_index++)
				for (ushort bit_index=0; (!(bit_index & 16)) && (byte_index << 4 | bit_index)<pagenum; bit_index++)
					if (!(b[byte_index] & (1 << bit_index)))
					{
						short slotID = _pageInsert(fileID, pageID + (byte_index << 4 | bit_index) + 1, objptr);
						if (slotID != -1)
							return RecordPos(pageID + (byte_index << 4 | bit_index) + 1, slotID);
						else
						{
							b = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
							b[byte_index] |= (1u << bit_index);
							bufManager->markDirty(bufIndex);
						}
					}
		}
		if (pagenum < FILE_STEP) break;
		pageID += FILE_STEP;
		b = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
		pagenum = b[PAGE_SSIZE-1];
	}

	// Check and add a flag page
	if (pagenum == FILE_STEP - 1)
	{
		b[PAGE_SSIZE-1] = FILE_STEP;
		bufManager->markDirty(bufIndex);
		pageID += FILE_STEP;
		b = (ushort*)(bufManager->allocPage(fileID, pageID, bufIndex, false));
		memset(b, 0, PAGE_SIZE);
		pagenum = 0;
	}

	// Add a new slot page
	b[PAGE_SSIZE-1] = ++pagenum;
	bufManager->markDirty(bufIndex);
	b = (ushort*)(bufManager->allocPage(fileID, pageID + pagenum, bufIndex, false));
	memset(b, 0, PAGE_SIZE);
	ushort slotID = _pageInsert(fileID, pageID + pagenum, objptr);
	return RecordPos(pageID + pagenum, slotID);
}

}	// end namespace SimpleDataBase
