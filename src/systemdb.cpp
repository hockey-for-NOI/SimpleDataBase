#include "systemdb.h"

namespace	SimpleDataBase
{

SystemDB*	SystemDB::m_instance = nullptr;

SystemDB&	SystemDB::get_instance()
{
	if (!m_instance) m_instance = new SystemDB();
	return *m_instance;
}

SystemDB::SystemDB()
{
	auto fm = std::make_shared<FileManager>();
	auto bfm = std::make_shared<BufPageManager>(fm);
	recordManager = new RecordManager(fm, bfm);
	indexManager = new IndexManager(fm, bfm);
}

}	// end namespace SimpleDataBase
