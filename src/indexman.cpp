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

RecordPos IndexManager::insert(int fileID, void const* objptr)
{
	ushort size; memcpy(&size, objptr, 2);
	int bufIndex;
	ushort* sb;
	BufType ib = bufManager->getPage(fileID, 0, bufIndex);
	RecordPos pos, tpos; memcpy(&pos, ((ushort*)objptr) + 1, 6);
	int p = ib[0]; int depth = ib[1] + 1;
	int pstack[depth];
	while (depth--)
	{
		pstack[depth] = p;
		sb = (ushort*)(bufManager->getPage(fileID, p, bufIndex));
		ushort sid;
		for (sid=1; sid<sb[0]; sid++)
		{
			memcpy(&tpos, sb + sid * 5 - 2, 6);
			if (pos < tpos) break;
		}
		memcpy(&p, sb + sid * 5 - 4, 4);
	}
	if (!_leafBinaryInsert(fileID, pageID, objptr))
	{
		sb = (ushort*)(bufManager->getPage(fileID, p, bufIndex));
		uchar* b = (uchar*)sb;

		std::vector <uchar> tPage0(PAGE_SIZE, 0), tPage1(PAGE_SIZE, 0);
		uchar* b0 = &tPage0[0], b1 = &tPage1[0];
		ushort* sb0 = (ushort*)b0, sb1 = (ushort*)b1;

		short islot, sum;
		for (islot=0, sum=0; sum<(empty_begin >> 1); islot++)
		{
			uchar* src = b + sb[PAGE_SSIZE-3-islot];
			ushort ssize; memcpy(&ssize, src, 2);
			memcpy(b0 + sum, b + src, ssize);
			sb0[PAGE_SSIZE-3-islot] = sum;
			sum += ssize;
		}
		sb0[PAGE_SSIZE-1] = sum;
		ushort slotsplit = sb0[PAGE_SSIZE-2] = islot;
		for (islot = sum = 0; slotsplit + islot < nslot; islot++)
		{
			uchar* src = b + sb[PAGE_SSIZE-3-slotsplit-islot];
			ushort ssize; memcpy(&ssize, src, 2);
			memcpy(b1 + sum, b + src, ssize);
			sb1[PAGE_SSIZE-3-islot] = sum;
			sum += ssize;
		}
		sb1[PAGE_SSIZE-1] = sum;
		sb1[PAGE_SSIZE-2] = islot;
		memcpy(&tpos, sb1 + 1, 6);
		memcpy(b, b0, PAGE_SIZE);
		bufManager->markDirty(bufIndex);
		if (pos < tpos) _leafBinaryInsert(fileID, p, objptr);

		ib = bufManager->getPage(fileID, 0, bufIndex);
		int newPage = ib[(PAGE_SIZE>>2)-1] ++;
		bufManager->markDirty(bufIndex);

		ib = bufManager->allocPage(fileID, newPage, bufIndex, false);
		memcpy(b, b1, PAGE_SIZE);
		bufManager->markDirty(bufIndex);
		if (!(pos < tpos)) _leafBinaryInsert(fileID, newPage, objptr);
		bufManager->writeBack(bufIndex);

		for (int i=0; i<depth; i++)
		{
			b = (uchar*)bufManager->getPage(fileID, stack[i], bufIndex);
			sb = (ushort*)b;
			//Insert (tpos, newPage)
			if (sb[0] == PAGE_PSIZE)
			{
				//Split and add a new newPage and update tpos
			}
			else break;
		}
	}
}

bool IndexManager::_leafBinaryInsert(int fileID, int pageID, void const* objptr);
{
	ushort size; memcpy(&size, objptr, 2);
	int bufIndex;
	ushort* sb = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
	uchar* b = (uchar*)sb;
	ushort &empty_begin = sb[PAGE_SSIZE - 1];
	ushort &nslot = sb[PAGE_SSIZE - 2];
	if (empty_begin + (nslot<<1) + 6 + size <= PAGE_SIZE)
	{
		memcpy(b + empty_begin, objptr, size);
		ushort id, sid;
		for (id=0, sid=PAGE_SSIZE-2-nslot; id<nslot; id++, sid--)
		{
			memcpy(&tpos, b + sb[sid] + 2, 6);
			if (pos < tpos) sb[sid - 1] = sb[sid]; else break;
		}
		sb[sid - 1] = empty_begin;
		empty_begin += size;
		bufManager->markDirty(bufIndex);
		return true;
	}
	else return false;
}

}	// end namespace SimpleDataBase
