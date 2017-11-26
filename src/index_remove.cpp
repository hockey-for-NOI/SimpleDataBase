#include "indexman.h"
#include <cstring>

namespace SimpleDataBase
{

int	IndexManager::remove(int fileID, RecordPos const& pos, std::function<bool(void const*)> const& confirm)
{
	int bufIndex;
	BufType ib = bufManager->getPage(fileID, 0, bufIndex);
	int root = ib[0], depth = ib[1];
	return _innerRemove(fileID, root, depth, pos, confirm);
}

int	IndexManager::_innerRemove(int fileID, int pageID, int depth, RecordPos const& pos, std::function<bool(void const*)> const& confirm)
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
	int subPage, tot = 0;
	for (ushort ir=rbegin; ir<=rend; ir++)
	{
		memcpy(&subPage, sb + ir * 5 + 1, 4);
		if (depth) tot += _innerRemove(fileID, subPage, depth-1, pos, confirm);
		else tot += _leafRemove(fileID, subPage, pos, confirm);
	}
	return tot;
}

int	IndexManager::_leafRemove(int fileID, int pageID, RecordPos const& pos, std::function<bool(void const*)> const& confirm)
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
		if (!confirm(b + sb[PAGE_SSIZE-3-rend]))
		{
			sb[PAGE_SSIZE-3-rbegin] = sb[PAGE_SSIZE-3-rend];
			rbegin++;
		}
	}
	if (rbegin < rend)
	{
		memmove(sb + PAGE_SSIZE - 2 - nslot + rend - rbegin,
				sb + PAGE_SSIZE - 2 - nslot,
				(nslot - rend) << 1);
		nslot -= rend - rbegin;
		bufManager->markDirty(bufIndex);
	}
	return rend - rbegin;
}

}	// end namespace SimpleDataBase
