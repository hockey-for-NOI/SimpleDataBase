#include "systemdb.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>

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

std::vector<Area>	SystemDB::getTableCols(std::string const& name)
{
	if (current_db == SYSTEM_DB_NAME || !current_db.length()) return std::vector<Area>();
	int fid = recordManager->openFile(getSysTable());
	std::vector<Area> result;
	for (auto const& i: recordManager->select<Area>(fid, [&name](Area const& area){return name == area.table;}))
		result.emplace_back(recordManager->get<Area>(fid, i));
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

bool	SystemDB::hasTable(std::string const& name)
{
	if (current_db == SYSTEM_DB_NAME || !current_db.length() || !name.length()) return 0;
	int fid = recordManager->openFile(getSysTable());
	bool flag = recordManager->select<Area>(fid, [&name](Area const& area){return name == area.table;}).size();
	recordManager->closeFile(fid);
	return flag;
}

bool	SystemDB::createTable(std::string const& name, std::vector<Area> const& areas)
{
	if (current_db == SYSTEM_DB_NAME || !current_db.length() || !name.length()) return 0;
	int fid = recordManager->openFile(getSysTable());
	bool	flag = 1;
	for (auto const& i: recordManager->select<Area>(fid, [&name](Area const& area){return name == area.table;}))
	{
		flag = 0;
		break;
	}
	if (!flag)
	{
		recordManager->closeFile(fid);
		return 0;
	}
	flag = 0;
	for (auto const& area: areas)
	{
		recordManager->ins<Area>(fid, area);
		if (area.primary) flag = 1;
	}
	recordManager->closeFile(fid);
	recordManager->createFile(getTableFile(name));
	if (flag) indexManager->createIndex(getIndexFile(name));
	return 1;
}

bool	SystemDB::insertRecord(std::string const& name, std::vector < std::vector <char> > const& data)
{
	if (current_db == SYSTEM_DB_NAME || !current_db.length() || !name.length()) return 0;
	int fid = recordManager->openFile(getSysTable());
	bool	hasprim = 0;
	Area	prim;
	for (auto const& i: recordManager->select<Area>(fid, [&name](Area const& area){return name == area.table;}))
	{
		prim = recordManager->get<Area>(fid, i);
		if (prim.primary) {hasprim = 1; break;}
	}
	recordManager->closeFile(fid);
	if (hasprim)
	{
		fid = indexManager->openIndex(getIndexFile(name));
		bool errflag = 0;
		for (auto const& dat: data)
		{
			if (dat[prim.offset]) {errflag = 1; break;}
			RecordPos key;
			if (prim.type == Area::INT_T)
			{
				int x;
				memcpy(&x, &dat[prim.offset + 1], 4);
				key.pageID = x; key.slotID = 0;
			}
			if (prim.type == Area::VARCHAR_T)
			{
				char tmp[prim.len + 1];
				memcpy(tmp, &dat[prim.offset + 1], prim.len);
				tmp[prim.len] = 0;
				key.pageID = std::hash<string>()(std::string(tmp));
				key.slotID = 0;
			}
			if (indexManager->get<int>(fid, key, [](void const*){return 1;}).size()) {errflag = 1; break;}
		}
		indexManager->closeIndex(fid);
		if (errflag) return 0;
	}
	std::vector <RecordPos> ps;
	fid = recordManager->openFile(getTableFile(name));
	for (auto const& dat: data) ps.push_back(recordManager->insert(fid, &dat[0]));
	recordManager->closeFile(fid);
	if (hasprim)
	{
		fid = indexManager->openIndex(getIndexFile(name));
		for (int id=0; id<data.size(); id++)
		{
			auto const& dat = data[id];
			auto const& pos = ps[id];
			RecordPos key;
			if (prim.type == Area::INT_T)
			{
				int x;
				memcpy(&x, &dat[prim.offset + 1], 4);
				key.pageID = x; key.slotID = 0;
			}
			if (prim.type == Area::VARCHAR_T)
			{
				char tmp[prim.len + 1];
				memcpy(tmp, &dat[prim.offset + 1], prim.len);
				tmp[prim.len] = 0;
				key.pageID = std::hash<string>()(std::string(tmp));
				key.slotID = 0;
			}
			indexManager->ins<NaiveIntIndex>(fid, NaiveIntIndex(key, pos));
		}
		indexManager->closeIndex(fid);
	}
	return 1;
}

int	SystemDB::deleteRecord(std::string const& name, std::function <bool(void const*)> const& cond)
{
	if (current_db == SYSTEM_DB_NAME || !current_db.length() || !name.length()) return 0;
	int fid = recordManager->openFile(getSysTable());
	bool	hasprim = 0;
	Area	prim;
	for (auto const& i: recordManager->select<Area>(fid, [&name](Area const& area){return name == area.table;}))
	{
		prim = recordManager->get<Area>(fid, i);
		if (prim.primary) {hasprim = 1; break;}
	}
	recordManager->closeFile(fid);
	fid = recordManager->openFile(getTableFile(name));
	int cnt = 0;
	std::vector <NaiveIntIndex> toremove;
	for (auto const& pos: recordManager->select(fid, cond))
	{
		char const* dat = (char const*)(recordManager->getptr(fid, pos));
		if (hasprim)
		{
			RecordPos key;
			if (prim.type == Area::INT_T)
			{
				int x;
				memcpy(&x, dat + prim.offset + 1, 4);
				key.pageID = x; key.slotID = 0;
			}
			if (prim.type == Area::VARCHAR_T)
			{
				char tmp[prim.len + 1];
				memcpy(tmp, dat + prim.offset + 1, prim.len);
				tmp[prim.len] = 0;
				key.pageID = std::hash<string>()(std::string(tmp));
				key.slotID = 0;
			}
			toremove.emplace_back(key, pos);
		}
		recordManager->remove(fid, pos), cnt++;
	}
	recordManager->closeFile(fid);
	if (hasprim)
	{
		fid = indexManager->openIndex(getIndexFile(name));
		for (auto const& i: toremove)
			indexManager->remove(fid, i.getkey(), [i](void const* ptr){
				return memcmp(ptr, &i, sizeof(NaiveIntIndex)) == 0;
			});
		indexManager->closeIndex(fid);
	}
	return cnt;
}

std::pair<int, int>	SystemDB::updateRecord(std::string const& name, std::function<bool(void const*)> const& cond, std::function< std::vector<char>(void const*) > const& upd)
{
	if (current_db == SYSTEM_DB_NAME || !current_db.length() || !name.length()) return std::make_pair(0, 0);
	int fid = recordManager->openFile(getTableFile(name));
	int cnt1 = 0, cnt2 = 0;
	std::vector < std::vector <char> > newdats;
	for (auto const& pos: recordManager->select(fid, cond))
	{
		void*	ptr = recordManager->getptr(fid, pos);
		auto newdat = upd(ptr);
		if (newdat.size()) newdats.push_back(newdat); else cnt2++;
	}
	recordManager->closeFile(fid);
	cnt1 = deleteRecord(name, cond);
	if (!insertRecord(name, newdats)) {cnt2 += cnt1; cnt1 = 0;}
	return std::make_pair(cnt1, cnt2);
}

std::vector < std::vector <char> >	SystemDB::selectRecord(std::string const& name, std::function<bool(void const*)> const& cond)
{
	std::vector < std::vector <char> > res;
	if (current_db == SYSTEM_DB_NAME || !current_db.length() || !name.length()) return res;
	int fid = recordManager->openFile(getTableFile(name));
	for (auto const& pos: recordManager->select(fid, cond))
	{
		void*	ptr = recordManager->getptr(fid, pos);
		unsigned short len;
		memcpy(&len, ptr, 2);
		std::vector <char> tmp(len);
		memcpy(&tmp[0], ptr, len);
		res.push_back(tmp);
	}
	recordManager->closeFile(fid);
	return res;
}

}	// end namespace SimpleDataBase
