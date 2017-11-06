#pragma once
#include "recordman.h"

namespace SimpleDataBase
{

class	IndexManager : protected RecordManager
{
public:
	static	const	int	PAGE_PSIZE = PAGE_SIZE / 5;
	using RecordManager::RecordManager;

	using RecordManager::getFileManager;
	using RecordManager::getBufPageManager;

	bool	createIndex(std::string const& indexName);
	inline	bool	removeIndex(std::string const& indexName) {removeFile(indexName);}
	inline	int	openIndex(std::string const& indexName) {return openFile(indexName);}
	inline	int closeIndex(int indexID) {return closeFile(indexID);}

	void	insert(int fileID, void const* objptr);
	template <typename T>
	RecordPos ins(int fileID, T const& obj) {insert(fileID, &obj);}
	int	remove(int fileID, RecordPos const& key);
	void*	getptr(int fileID, RecordPos const& key);

protected:
	bool _leafBinaryInsert(int fileID, int pageID, void const* objptr);
	int	_innerRemove(int fileID, int pageID, int depth, RecordPos const& pos);
	int	_leafRemove(int fileID, int pageID, RecordPos const& pos);
};

}	// end namespace SimpleDataBase
