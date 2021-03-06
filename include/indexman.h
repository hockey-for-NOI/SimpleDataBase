#pragma once
#include "recordman.h"
#include <cstring>

namespace SimpleDataBase
{

struct	NaiveIntIndex
{
	unsigned short	size, key[3], pos[3];
	inline	NaiveIntIndex(RecordPos const& _key, RecordPos const& _pos)
	{
		size = 14;
		memcpy(key, &_key, 6);
		memcpy(pos, &_pos, 6);
	}
	inline	RecordPos	getkey() const
	{
		RecordPos _key;
		memcpy(&_key, key, 6);
		return _key;
	}
	inline	RecordPos	getpos() const
	{
		RecordPos _pos;
		memcpy(&_pos, pos, 6);
		return _pos;
	}
};

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
	void	ins(int fileID, T const& obj) {insert(fileID, &obj);}
	int		remove(int fileID, RecordPos const& key, std::function<bool(void const*)> const& confirm = 
				[](void const*){return 1;});
	void	getobj(int fileID, RecordPos const& key, std::function<void(void const*)> const& grabber);
	template <typename T>
	std::vector<T>	get(int fileID, RecordPos const& key, std::function<T(void const*)> const& grabber)
	{
		std::vector<T> res(0);
		getobj(fileID, key, [&res, &grabber](void const* p){res.emplace_back(grabber(p));});
		return res;
	}

protected:
	bool _leafBinaryInsert(int fileID, int pageID, void const* objptr);
	int	_innerRemove(int fileID, int pageID, int depth, RecordPos const& pos, std::function<bool(void const*)> const& confirm);
	int	_leafRemove(int fileID, int pageID, RecordPos const& pos, std::function<bool(void const*)> const& confirm);
	void _innerGetobj(int fileID, int pageID, int depth, RecordPos const& pos, std::function<void(void const*)> const& grabber);
	void _leafGetobj(int fileID, int pageID, RecordPos const& pos, std::function<void(void const*)> const& grabber);
};

}	// end namespace SimpleDataBase
