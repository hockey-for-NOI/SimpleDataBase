#pragma once
#include "indexman.h"

namespace	SimpleDataBase
{

class	SystemDB
{

public:
	SystemDB&	get_instance();
	inline	std::shared_ptr<RecordManager>	getRecordManager() {return recordManager;}
	inline	std::shared_ptr<IndexManager>	getIndexManager() {return indexManager;}

private:
	SystemDB();

	std::shared_ptr<RecordManager> recordManager;
	std::shared_ptr<IndexManager> indexManager;

	static	SystemDB*	m_instance;
};

}	// end namespace SimpleDataBase
