#pragma once
#include "bufmanager/BufPageManager.h"
#include "fileio/FileManager.h"
#include "utils/pagedef.h"

#include <functional>

namespace SimpleDataBase
{

struct	RecordPos
{
	size_t pageID;
	ushort slotID;
};

class	RecordMananger
{
public:
	RecordManager() = delete;

	inline	RecordManager(
			FileManager* fileManager
			BufPageManager* bufManager):
		fileManager(fileManager),
		bufManager(bufManager)
	{}

	bool	createFile(std::string fileName);
	bool	removeFile(std::string fileName);
	int	openFile(std::string fileName);
	int	closeFile(int fileID);

	RecordPos	insert(void const* objptr, size_t size);
	template <typename T>
	void insert(T const& obj) {insert(&obj, sizeof(T));}
	void	remove(RecordPos const& pos);
	void*	getptr(RecordPos const& pos);
	template <typename T>
	T&	get(RecordPos const& pos) {return *((T*)getptr(pos));}
	void	update(RecordPos const& pos, void const* objptr, size_t size);
	template <typename T>
	void	update(RecordPos const& pos, T const& obj) {update(pos, &obj, sizeof(T));}
	std::vector<RecordPos>	select(std::function<bool(void const*)> const& fun);
	template <typename T>
	std::vector<RecordPos>	select(std::function<bool(T const&)> const& fun)
	{return select([&fun](void const* ptr){return fun(*((T const*)ptr))});}

private:
	BufPageManager* bufManager;
	FileManager* fileManager;
};

}	// end namespace SimpleDataBase
