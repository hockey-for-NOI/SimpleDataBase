#include "systemdb.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>

namespace	SimpleDataBase
{

const std::string SystemDB::DEFAULT_ROOT_DIR = "/data/db";
const std::string SystemDB::SYSTEM_DB_NAME = "system";

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
	recordManager = std::make_shared<RecordManager>(fm, bfm);
	indexManager = std::make_shared<IndexManager>(fm, bfm);

	current_db = SYSTEM_DB_NAME;
	FILE *f = fopen(getCurrentDir().c_str(), "a");
	if (f)
	{
		fclose(f);
		system(("rm " + getCurrentDir()).c_str());
		system(("mkdir -p " + getCurrentDir()).c_str());
	}
	current_db = "";
}

bool	SystemDB::createDB(std::string const& name)
{
	if (name == SYSTEM_DB_NAME || !name.length()) return 0;
	std::string prev_db = current_db;
	current_db = SYSTEM_DB_NAME;

	FILE *f = fopen(getTableFile(name).c_str(), "r");
	if (f) {fclose(f); current_db = prev_db; return 0;}

	recordManager->createFile(getTableFile(name));
	current_db = name;
	system(("mkdir -p " + getCurrentDir()).c_str());
	
	current_db = prev_db;
	return 1;
}

bool	SystemDB::dropDB(std::string const& name)
{
	if (name == SYSTEM_DB_NAME || !name.length()) return 0;
	std::string prev_db = current_db;
	current_db = SYSTEM_DB_NAME;

	FILE *f = fopen(getTableFile(name).c_str(), "r");
	if (!f) {current_db = prev_db; return 0;}
	fclose(f);
	system(("rm " + getTableFile(name)).c_str());
	current_db = name;
	system(("rm -rf " + getCurrentDir()).c_str());

	current_db = prev_db == name ? "" : prev_db;
	return 1;
}

std::string SystemDB::showDB()
{
	static	const	std::string tmpfile = "/tmp/mydb.showdb.tmp.0001";

	std::string prev_db = current_db;
	current_db = SYSTEM_DB_NAME;
	system(("ls " + getCurrentDir() + " > " + tmpfile).c_str());
	current_db = prev_db;

	std::ifstream f(tmpfile);
	return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

bool	SystemDB::useDB(std::string const& name)
{
	if (name == SYSTEM_DB_NAME || !name.length()) return 0;
	std::string prev_db = current_db;
	current_db = SYSTEM_DB_NAME;

	FILE *f = fopen(getTableFile(name).c_str(), "r");
	if (!f) {current_db = prev_db; return 0;}
	fclose(f);
	current_db = name;
	return 1;
}

std::string	SystemDB::showTable()
{
	if (current_db == SYSTEM_DB_NAME || !current_db.length()) return "No table using.";
	int fid = recordManager->openFile(getSysTable());
	std::string prev_table = "";
	std::string result;
	for (auto const& i: recordManager->select(fid, [](void const*){return true;}))
	{
		Area const& area = recordManager->get<Area>(fid, i);
		if (std::string(area.table) != prev_table)
		{
			result = result + area.table + ":\n";
			prev_table = area.table;
		}
		result = result + "\t" + area.name + ": " + area.showtype();
		if (area.notnull) result = result + " NOT NULL";
		if (area.primary) result = result + " PRIMARY KEY";
		result = result + "\n";
	}
	recordManager->closeFile(fid);
	return result;
}

bool	SystemDB::dropTable(std::string const& name)
{
	if (current_db == SYSTEM_DB_NAME || !current_db.length() || !name.length()) return 0;
	int fid = recordManager->openFile(getSysTable());
	bool	flag = 0;
	for (auto const& i: recordManager->select<Area>(fid, [&name](Area const& area){return name == area.table;}))
	{
		recordManager->remove(fid, i);
		flag = 1;
	}
	recordManager->closeFile(fid);
	if (flag)
	{
		system(("rm " + getTableFile(name)).c_str());
	}
	return flag;
}

bool	SystemDB::createTable(std::string const& name, std::vector<Area> const& areas)
{
	if (current_db == SYSTEM_DB_NAME || !current_db.length() || !name.length()) return 0;
	int fid = recordManager->openFile(getSysTable());
	bool	flag = 1;
	for (auto const& i: recordManager->select<Area>(fid, [&name](Area const& area){return name == area.name;}))
	{
		flag = 0;
		break;
	}
	if (!flag)
	{
		recordManager->closeFile(fid);
		return 0;
	}
	for (auto const& area: areas)
		recordManager->ins<Area>(fid, area);
	recordManager->closeFile(fid);
	recordManager->createFile(getTableFile(name));
	return 1;
}

}	// end namespace SimpleDataBase
