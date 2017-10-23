#include "recordman.h"

namespace SimpleDataBase
{

void*	RecordManager::getptr(int fileID, RecordPos const& p)
{
	int bufIndex;
	uchar* b = (uchar*)(bufManager->getPage(fileID, p.pageID, bufIndex));
	ushort* sb = (ushort*)b;
	return b + (p.slotID ? sb[PAGE_SSIZE - p.slotID - 1] : 0);
}

std::vector<RecordPos> RecordManager::select(int fileID, std::function<bool(void const*)> const& fun)
{
	std::vector <RecordPos> res(0);
	int bufIndex, pageID = 0;
	ushort* b = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
	ushort nPage = b[PAGE_SSIZE - 1];
	while (true)
	{
		for (ushort i=0; i<nPage; i++)
		{
			auto subres = _pageSelect(fileID, pageID + i + 1, fun);
			res.insert(res.end(), subres.begin(), subres.end());
		}
		if (nPage < FILE_STEP) break;
		b = (ushort*)(bufManager->getPage(fileID, pageID += FILE_STEP, bufIndex));
		nPage = b[PAGE_SSIZE - 1];
	}
	return res;
}

std::vector<RecordPos> RecordManager::_pageSelect(int fileID, int pageID, std::function<bool(void const*)> const& fun)
{
	int bufIndex;
	uchar* b = (uchar*)(bufManager->getPage(fileID, pageID, bufIndex));
	ushort* sb = (ushort*)b; 
	ushort nSlot = sb[PAGE_SSIZE-1];
	std::vector <RecordPos> res; res.reserve(nSlot);
	if (nSlot && sb[PAGE_SSIZE - 2] && fun(b))
		res.emplace_back(pageID, 0);
	ushort last = 0;
	for (ushort i=1; i<nSlot; i++)
		if (sb[PAGE_SSIZE - i - 1] != last)
		{
			last = sb[PAGE_SSIZE - i - 1];
			if (fun(b + sb[PAGE_SSIZE - i - 1]))
				res.emplace_back(pageID, i);
		}
	return res;
}

}	// end namespace SimpleDataBase
