#include "indexman.h"
#include <cstring>

namespace SimpleDataBase
{

void IndexManager::insert(int fileID, void const* objptr)
{
	ushort size; memcpy(&size, objptr, 2);
	int bufIndex;
	ushort* sb;
	BufType ib = bufManager->getPage(fileID, 0, bufIndex);
	RecordPos pos, tpos; memcpy(&pos, ((ushort*)objptr) + 1, 6);
	int p = ib[0]; int depth = ib[1] + 1;
	int pstack[depth];
	for (int idp=depth; idp--;)
	{
		pstack[idp] = p;
		sb = (ushort*)(bufManager->getPage(fileID, p, bufIndex));
		ushort sid;
		for (sid=1; sid<sb[0]; sid++)
		{
			memcpy(&tpos, sb + sid * 5 - 2, 6);
			if (pos < tpos) break;
		}
		memcpy(&p, sb + sid * 5 - 4, 4);
	}
	if (!_leafBinaryInsert(fileID, p, objptr))
	{
		sb = (ushort*)(bufManager->getPage(fileID, p, bufIndex));
		uchar* b = (uchar*)sb;

		std::vector <uchar> tPage0(PAGE_SIZE, 0), tPage1(PAGE_SIZE, 0);
		uchar* b0 = &tPage0[0], *b1 = &tPage1[0];
		ushort* sb0 = (ushort*)b0, *sb1 = (ushort*)b1;

		short islot, sum;
		for (islot=0, sum=0; sum<(sb[PAGE_SSIZE-1] >> 1); islot++)
		{
			uchar* src = b + sb[PAGE_SSIZE-3-islot];
			ushort ssize; memcpy(&ssize, src, 2);
			memcpy(b0 + sum, src, ssize);
			sb0[PAGE_SSIZE-3-islot] = sum;
			sum += ssize;
		}
		sb0[PAGE_SSIZE-1] = sum;
		ushort slotsplit = sb0[PAGE_SSIZE-2] = islot;
		for (islot = sum = 0; slotsplit + islot < sb[PAGE_SSIZE-2]; islot++)
		{
			uchar* src = b + sb[PAGE_SSIZE-3-slotsplit-islot];
			ushort ssize; memcpy(&ssize, src, 2);
			memcpy(b1 + sum, src, ssize);
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
		memcpy(ib, b1, PAGE_SIZE);
		bufManager->markDirty(bufIndex);
		if (!(pos < tpos)) _leafBinaryInsert(fileID, newPage, objptr);

		int idepth;
		for (idepth=0; idepth<depth; idepth++)
		{
			b = (uchar*)bufManager->getPage(fileID, pstack[idepth], bufIndex);
			sb = (ushort*)b;
			//Insert (tpos, newPage)
			ushort islot;
			for (islot=sb[0]; --islot; )
			{
				memcpy(&pos, sb + islot * 5 - 2, 6);
				if (pos < tpos) break;
				else memcpy(sb + islot * 5 + 3, sb + islot * 5 - 2, 6);
			}
			memcpy(sb + islot * 5 + 3, &tpos, 6);
			memcpy(sb + islot * 5 + 6, &newPage, 4);
			bufManager->markDirty(bufIndex);
			if (PAGE_PSIZE == ++sb[0])
			{
				//Split and add a new newPage and update tpos
				sb1[0] = PAGE_PSIZE >> 1;
				memcpy(sb1 + 1, sb + sb1[0] * 5 + 6, PAGE_PSIZE * 5 - 6);
				sb[0] = (PAGE_PSIZE >> 1) | 1;
				memcpy(&tpos, sb + sb[0] * 5 + 6, 6);
				
				ib = bufManager->getPage(fileID, 0, bufIndex);
				newPage = ib[(PAGE_SIZE>>2)-1] ++;
				bufManager->markDirty(bufIndex);

				b = (uchar*)bufManager->getPage(fileID, newPage, bufIndex);
				memcpy(b, sb1, PAGE_PSIZE * 5 - 4);
				bufManager->markDirty(bufIndex);
			}
			else break;
		}
		if (idepth == depth)
		{
			//Create newPage and reset root
			ib = bufManager->getPage(fileID, 0, bufIndex);
			p = ib[0] = ib[(PAGE_SIZE>>2)-1] ++;
			++ib[1];
			bufManager->markDirty(bufIndex);

			b = (uchar*)bufManager->getPage(fileID, p, bufIndex);
			sb = (ushort*)b;
			sb[0] = 2;
			memcpy(sb + 1, pstack + depth - 1, 4);
			memcpy(sb + 3, &tpos, 6);
			memcpy(sb + 6, &newPage, 4);
			bufManager->markDirty(bufIndex);
		}
	}
}

bool IndexManager::_leafBinaryInsert(int fileID, int pageID, void const* objptr)
{
	ushort size; memcpy(&size, objptr, 2);
	int bufIndex;
	ushort* sb = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
	uchar* b = (uchar*)sb;
	ushort &empty_begin = sb[PAGE_SSIZE - 1];
	ushort &nslot = sb[PAGE_SSIZE - 2];
	RecordPos pos, tpos; memcpy(&pos, ((ushort*)objptr) + 1, 6);
	if (empty_begin + (nslot<<1) + 6 + size <= PAGE_SIZE)
	{
		memcpy(b + empty_begin, objptr, size);
		ushort id, sid;
		for (id=0, sid=PAGE_SSIZE-2-nslot; id<nslot; id++, sid++)
		{
			memcpy(&tpos, b + sb[sid] + 2, 6);
			if (pos < tpos) sb[sid - 1] = sb[sid]; else break;
		}
		sb[sid - 1] = empty_begin;
		empty_begin += size;
		nslot++;
		bufManager->markDirty(bufIndex);
		return true;
	}
	else return false;
}

}	// end namespace SimpleDataBase
