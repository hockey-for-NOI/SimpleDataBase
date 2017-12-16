#pragma once
#include "indexman.h"
#include <string>
#include <vector>

namespace	SimpleDataBase
{

class	SystemDB
{

public:
	static	const	std::string	DEFAULT_ROOT_DIR;
	static	const	std::string SYSTEM_DB_NAME;

	SystemDB&	get_instance();
	inline	std::shared_ptr<RecordManager>	getRecordManager() {return recordManager;}
	inline	std::shared_ptr<IndexManager>	getIndexManager() {return indexManager;}

	inline	std::string	getCurrentDir() {return DEFAULT_ROOT_DIR + "/" + current_db;}
	inline	std::string getTableFile(std::string table) {return getCurrentDir() + "/" + table;}

	bool	createDB(std::string const& name);
	bool	dropDB(std::string const& name);
	std::string	showDB();
	bool	useDB(std::string const& name);

private:
	SystemDB();

	std::shared_ptr<RecordManager> recordManager;
	std::shared_ptr<IndexManager> indexManager;

	std::string current_db;

	static	SystemDB*	m_instance;
};

}	// end namespace SimpleDataBase
