#pragma once
#include "recordman.h"

namespace SimpleDataBase
{

class	IndexManager : private RecordManager
{
public:
	using RecordManager::RecordManager;

	using RecordManager::getFileManager;
	using RecordManager::getBufPageManager;

	inline	bool	createIndex(std::string const& indexName);
	inline	bool	removeIndex(std::string const& indexName) {removeFile(indexName);}
	inline	int	openIndex(std::string const& indexName) {return openFile(indexName);}
	inline	int closeIndex(int indexID) {return closeFile(indexID);}

	RecordPos	insert(int fileID, void const* objptr, size_t size);
	template <typename T>
	RecordPos	insert(int fileID, T const& obj) {insert(fileID, &obj, sizeof(T));}
	void	remove(int fileID, RecordPos const& key);
	void*	getptr(int fileID, RecordPos const& key);
};

}	// end namespace SimpleDataBase
