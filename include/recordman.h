#pragma once
#include "bufmanager/BufPageManager.h"
#include "fileio/FileManager.h"
#include "utils/pagedef.h"

#include <vector>
#include <functional>

namespace SimpleDataBase
{

struct	RecordPos
{
	size_t pageID;
	ushort slotID;

	RecordPos() = default;
	RecordPos(int pageID, ushort slotID):
		pageID(pageID), slotID(slotID) {}
};

class	RecordManager
{
public:
	static	const	ushort	FILE_STEP = 65520u;

	RecordManager();
	~RecordManager();

	bool	createFile(std::string fileName);
	bool	removeFile(std::string fileName);
	int	openFile(std::string fileName);
	int	closeFile(int fileID);

	RecordPos	insert(int fileID, void const* objptr, size_t size);
	template <typename T>
	RecordPos insert(int fileID, T const& obj) {insert(fileID, &obj, sizeof(T));}
	void	remove(int fileID, RecordPos const& pos);
	void*	getptr(int fileID, RecordPos const& pos);
	template <typename T>
	T&	get(int fileID, RecordPos const& pos) {return *((T*)getptr(fileID, pos));}
	void	update(int fileID, RecordPos const& pos, void const* objptr, size_t size);
	template <typename T>
	void	update(int fileID, RecordPos const& pos, T const& obj) {update(fileID, pos, &obj, sizeof(T));}
	std::vector<RecordPos>	select(int fileID, std::function<bool(void const*)> const& fun);
	template <typename T>
	std::vector<RecordPos>	select(int fileID, std::function<bool(T const&)> const& fun)
	{return select(fileID, [&fun](void const* ptr){return fun(*((T const*)ptr));});}

private:
	BufPageManager* bufManager;
	FileManager* fileManager;

	ushort	_pageInsert(int fileID, int pageID, void const* objptr, size_t size);
};

}	// end namespace SimpleDataBase
