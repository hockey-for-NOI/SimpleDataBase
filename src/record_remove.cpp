#include "recordman.h"
#include <cstring>

namespace	SimpleDataBase
{

void	RecordManager::remove(int fileID, RecordPos const& p)
{
	// Shift, and set next ptr to this ptr.
	// Waste 16 bits.
	ushort pageShift = p.pageID % FILE_STEP;
	unsigned flagPageID = p.pageID - pageShift;
	int bufIndex;
	
	uchar* b = (uchar*)(bufManager->getPage(fileID, p.pageID, bufIndex));
	short* sb = (short*)b;

	short &pos = sb[PAGE_SSIZE - p.slotID - 3];
	if (pos == -1) return;

	short dataShift = short(b[pos + 1]) << 8 | short(b[pos]);
	short &nSlot = sb[PAGE_SSIZE - 2];
	short &tail = sb[PAGE_SSIZE - 1];
	for (short i=PAGE_SSIZE-3; i>PAGE_SSIZE-nSlot-3; i--)
		if (sb[i] > pos) sb[i] -= dataShift;

	memmove(b + pos, b + pos + dataShift, tail - pos - dataShift);
	tail -= dataShift;
	pos = -1;
	bufManager->markDirty(bufIndex);

	BufType flagb = bufManager->getPage(fileID, flagPageID, bufIndex);
	flagb[(pageShift-1) >> 5] &= ~(1u << ((pageShift-1) & 31));
	bufManager->markDirty(bufIndex);
}

}	// end namespace SimpleDataBase
