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
	unsigned pageID;
	short slotID;

	RecordPos() = default;
	RecordPos(unsigned pageID, short slotID):
		pageID(pageID), slotID(slotID) {}
};

class	RecordManager
{
public:
	static	const	ushort	FILE_STEP = 65520u;
	static	const	short	PAGE_SSIZE = PAGE_SIZE >> 1;

	RecordManager(std::shared_ptr<FileManager> fileManager = nullptr,
			std::shared_ptr<BufPageManager> bufManager = nullptr);

	inline	std::shared_ptr<FileManager> getFileManager() const {return fileManager;}
	inline	std::shared_ptr<BufPageManager> getBufPageManager() const {return bufManager;}

	bool	createFile(std::string const& fileName);
	bool	removeFile(std::string const& fileName);
	int	openFile(std::string const& fileName);
	int	closeFile(int fileID);

	RecordPos	insert(int fileID, void const* objptr, size_t size);
	template <typename T>
	RecordPos insert(int fileID, T const& obj) {insert(fileID, &obj, sizeof(T));}
	void	remove(int fileID, RecordPos const& pos);
	void*	getptr(int fileID, RecordPos const& pos);
	template <typename T>
	T&	get(int fileID, RecordPos const& pos) {return *((T*)getptr(fileID, pos));}
	std::vector<RecordPos>	select(int fileID, std::function<bool(void const*)> const& fun);
	template <typename T>
	std::vector<RecordPos>	select(int fileID, std::function<bool(T const&)> const& fun)
	{return select(fileID, [&fun](void const* ptr){return fun(*((T const*)ptr));});}

protected:
	std::shared_ptr<BufPageManager> bufManager;
	std::shared_ptr<FileManager> fileManager;

	short	_pageInsert(int fileID, unsigned pageID, void const* objptr, size_t size);
	std::vector<short>	_pageSelect(int fileID, unsigned pageID, std::function<bool(void const*)> const& fun);
};

}	// end namespace SimpleDataBase
