#include "recordman.h"

namespace SimpleDataBase
{

void*	RecordManager::getptr(int fileID, RecordPos const& p)
{
	int bufIndex;
	uchar* b = (uchar*)(bufManager->getPage(fileID, p.pageID, bufIndex));
	short* sb = (short*)b;

	short &pos = sb[PAGE_SSIZE - p.slotID - 2];
	if (pos == -1) return nullptr;
	return b + pos;
}

std::vector<RecordPos> RecordManager::select(int fileID, std::function<bool(void const*)> const& fun)
{
	std::vector <RecordPos> res(0);
	int bufIndex, pageID = 0;
	ushort* b = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
	ushort nPage = b[PAGE_SSIZE - 1];
	while (true)
	{
		for (ushort i=pageID+1; i<=pageID+nPage; i++)
			for (auto slotID: _pageSelect(fileID, i, fun))
				res.emplace_back(i, slotID);
		if (nPage < FILE_STEP) break;
		b = (ushort*)(bufManager->getPage(fileID, pageID += FILE_STEP, bufIndex));
		nPage = b[PAGE_SSIZE - 1];
	}
	return res;
}

std::vector<short> RecordManager::_pageSelect(int fileID, int pageID, std::function<bool(void const*)> const& fun)
{
	int bufIndex;
	uchar* b = (uchar*)(bufManager->getPage(fileID, pageID, bufIndex));
	short* sb = (short*)b; 
	short nSlot = sb[PAGE_SSIZE-2];
	std::vector <short> res; res.reserve(nSlot);
	for (short i=0; i<nSlot; i++)
	{
		short& pos = sb[PAGE_SSIZE - i - 3];
		if (pos != -1) if (fun(b + pos))
			res.push_back(i);
	}
	return res;
}

}	// end namespace SimpleDataBase
