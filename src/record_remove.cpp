#include "recordman.h"
#include <cstring>

namespace	SimpleDataBase
{

void	RecordManager::remove(int fileID, RecordPos const& p)
{
	// Shift, and set next ptr to this ptr.
	// Waste 16 bits.
	int pageShift = p.pageID % FILE_STEP;
	int flagPageID = p.pageID - pageShift;
	int bufIndex;
	BufType flagb = bufManager->getPage(fileID, flagPageID, bufIndex);
	flagb[(pageShift-1) >> 5] &= ~(1u << ((pageShift-1) & 31));
	bufManager->markDirty(bufIndex);
	
	ushort* b = (ushort*)(bufManager->getPage(fileID, p.pageID, bufIndex));

	ushort dataShift = b[PAGE_SSIZE - p.slotID - 2] - (p.slotID ? b[PAGE_SSIZE - p.slotID - 1] : 0);
	ushort nSlot = b[PAGE_SSIZE - 1];
	for (ushort i=PAGE_SSIZE-p.slotID-2; i>PAGE_SSIZE-nSlot-2; i--)
		b[i] -= dataShift;

	memmove(((uchar*)b) + b[PAGE_SSIZE - p.slotID - 2], ((uchar*)b) + b[PAGE_SSIZE - p.slotID - 2] + dataShift, b[PAGE_SSIZE-nSlot-1]-b[PAGE_SSIZE - p.slotID - 2]);

	bufManager->markDirty(bufIndex);
}

}	// end namespace SimpleDataBase
