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
	recordManager = new RecordManager(fm, bfm);
	indexManager = new IndexManager(fm, bfm);

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

	File *f = fopen(getTableFile(name).c_str(), "r");
	if (f) {fclose(f); return 0;}

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

	File *f = fopen(getTableFile(name).c_str(), "r");
	if (!f) return 0;
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

bool	SystemDB::useDB(std::string name)
{
	if (name == SYSTEM_DB_NAME || !name.length()) return 0;
	std::string prev_db = current_db;
	current_db = SYSTEM_DB_NAME;

	File *f = fopen(getTableFile(name).c_str(), "r");
	if (!f) return 0;
	fclose(f);
	current_db = name;
	return 1;
}

}	// end namespace SimpleDataBase
