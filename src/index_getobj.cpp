#include "indexman.h"
#include <cstring>

namespace SimpleDataBase
{

void	IndexManager::getobj(int fileID, RecordPos const& pos, std::function<void(void const*)> const& grabber)
{
	int bufIndex;
	BufType ib = bufManager->getPage(fileID, 0, bufIndex);
	int root = ib[0], depth = ib[1];
	_innerGetobj(fileID, root, depth, pos, grabber);
}

void	IndexManager::_innerGetobj(int fileID, int pageID, int depth, RecordPos const& pos, std::function<void(void const*)> const& grabber)
{
	int bufIndex;
	ushort* sb = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
	ushort &nslot = sb[0];
	ushort rbegin, rend;
	RecordPos tpos;
	for (rbegin=0; rbegin+1<nslot; rbegin++)
	{
		memcpy(&tpos, sb + rbegin * 5 + 3, 6);
		if (!(tpos < pos)) break;
	}
	for (rend=rbegin; rend+1<nslot; rend++)
	{
		memcpy(&tpos, sb + rbegin * 5 + 3, 6);
		if (pos < tpos) break;
	}
	int subPage;
	for (ushort ir=rbegin; ir<=rend; ir++)
	{
		memcpy(&subPage, sb + ir * 5 + 1, 4);
		if (depth) _innerGetobj(fileID, subPage, depth-1, pos, grabber);
		else _leafGetobj(fileID, subPage, pos, grabber);
	}
}

void	IndexManager::_leafGetobj(int fileID, int pageID, RecordPos const& pos, std::function<void(void const*)> const& grabber)
{
	int bufIndex;
	ushort* sb = (ushort*)(bufManager->getPage(fileID, pageID, bufIndex));
	uchar* b = (uchar*)sb;
	ushort rbegin, rend;
	ushort &nslot = sb[PAGE_SSIZE-2];
	RecordPos tpos;
	for (rbegin=0; rbegin<nslot; rbegin++)
	{
		memcpy(&tpos, b + sb[PAGE_SSIZE-3-rbegin] + 2, 6);
		if (!(tpos < pos)) break;
	}
	for (rend=rbegin; rend<nslot; rend++)
	{
		memcpy(&tpos, b + sb[PAGE_SSIZE-3-rend] + 2, 6);
		if (pos < tpos) break;
		grabber(b + sb[PAGE_SSIZE-3-rend]);
	}
}

}	// end namespace SimpleDataBase
