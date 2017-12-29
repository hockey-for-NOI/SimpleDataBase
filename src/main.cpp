#include <iostream>
#include <string>
#include <cstring>
#include "systemdb.h"

#include "SQLParser.h"
#include <map>

using	namespace	std;

int	main()
{
	cout << "Loading..."; cout.flush();
	auto	&sys = SimpleDataBase::SystemDB::get_instance();
	cout << "Done." << endl;

	while (true)
	{
		cout << "[" << sys.getCurrentDB() << "]>"; cout.flush();
		std::string input_str;
		getline(cin, input_str);
		auto result = hsql::SQLParser::parseSQLString(input_str);

		if (result->isValid) for (int rid=0; rid<result->size(); rid++)
		{
			auto const& stmt = result->statements[rid];
			switch (stmt->type())
			{
				case hsql::kStmtCreate:
				{
					auto cstmt = dynamic_cast<hsql::CreateStatement*>(stmt);
					switch (cstmt->type)
					{
						case hsql::CreateStatement::kDatabase:
							SimpleDataBase::SystemDB::get_instance().createDB(cstmt->tableName);
						break;
						case hsql::CreateStatement::kTable:
							auto &cols = *cstmt->columns;
							std::map <string, SimpleDataBase::Area> areas;
							string	prim = "";
							for (int i=0; i<cols.size(); i++)
							{
								if (cols[i]->type == hsql::ColumnDefinition::PRIMARY)
								{
									if (prim.length()) {cout << "ERROR: Duplicate Primary Key." << endl; break;}
									prim = string(cols[i]->name);
									continue;
								}
								if (areas.count(string(cols[i]->name))) {cout << "ERROR: Duplicate Key." << endl; break;}
								auto &area = areas[string(cols[i]->name)];
								switch (cols[i]->type)
								{
									case hsql::ColumnDefinition::INT:
										area.type = SimpleDataBase::Area::INT_T;
									break;
									case hsql::ColumnDefinition::VARCHAR:
										area.type = SimpleDataBase::Area::VARCHAR_T;
									break;
								}
								area.len = cols[i]->len;
								area.size = sizeof(SimpleDataBase::Area);
								strcpy(area.name, cols[i]->name);
								strcpy(area.table, cstmt->tableName);
								area.notnull = cols[i]->notnull ? 1 : 0;
								area.primary = 0;
							}
							if (prim.length())
							{
								if (!areas.count(prim)) {cout << "ERROR: Primary Key Not Found." << endl; break;}
								areas[prim].primary = 1;
								areas[prim].notnull = 1;
							}
							std::vector <SimpleDataBase::Area> areas_vec;
							unsigned short offset = 2;
							for (int id=0; id<cols.size(); id++) if (cols[id]->type != hsql::ColumnDefinition::PRIMARY)
							{
								auto const& i = areas[std::string(cols[id]->name)];
								areas_vec.push_back(i);
								areas_vec.back().offset = offset;
								offset += areas_vec.back().len + 1;
							}
							if (!SimpleDataBase::SystemDB::get_instance().createTable(cstmt->tableName, areas_vec)) {cout << "ERROR: Create Table Failed." << endl;}
						break;
					}
				}
				break;
				case hsql::kStmtDrop:
				{
					auto dstmt = dynamic_cast<hsql::DropStatement*>(stmt);
					switch (dstmt->type)
					{
						case hsql::DropStatement::kDatabase:
							if (!SimpleDataBase::SystemDB::get_instance().dropDB(dstmt->name)) {cout << "ERROR: Drop Database Failed." << endl;}
						break;
						case hsql::DropStatement::kTable:
							if (!SimpleDataBase::SystemDB::get_instance().dropTable(dstmt->name)) {cout << "ERROR: Drop Table Failed." << endl;}
						break;
					}
				}
				break;
				case hsql::kStmtUse:
				{
					auto ustmt = dynamic_cast<hsql::UseStatement*>(stmt);
					if (!SimpleDataBase::SystemDB::get_instance().useDB(ustmt->name)) {cout << "ERROR: Use Table Failed." << endl;}
				}
				break;
				case hsql::kStmtShow:
				{
					auto sstmt = dynamic_cast<hsql::ShowStatement*>(stmt);
					switch (sstmt->type)
					{
						case hsql::ShowStatement::kDatabase:
							cout << SimpleDataBase::SystemDB::get_instance().showDB() << endl;
						break;
						case hsql::ShowStatement::kTable:
							cout << SimpleDataBase::SystemDB::get_instance().showTable() << endl;
						break;
					}
				}
				break;
				case hsql::kStmtInsert:
				{
					auto &sys = SimpleDataBase::SystemDB::get_instance();
					if (!sys.getCurrentDB().length()) {cout << "ERROR: No current DB." << endl; break;}
					auto istmt = dynamic_cast<hsql::InsertStatement*>(stmt);
					auto tablename = std::string(istmt->tableName);
					if (!sys.hasTable(tablename)) {cout << "ERROR: No such table." << endl; break;}
					std::vector < std::vector<char> > chardata;
					auto cols = sys.getTableCols(tablename);
					auto &dats = *istmt->values;
					bool	flag = 0;
					for (auto datp: dats)
					{
						auto &dat = *datp;
						if (cols.size() != dat.size()) {cout << "Column number mismatch." << endl; flag = 1; break;}
						std::vector <char> tot;
						tot.resize(2);
						for (int i=0; i<cols.size(); i++)
						{
							std::vector <char> buffer;
							switch (dat[i]->type)
							{
								case hsql::kExprLiteralInt:
									if (cols[i].type == SimpleDataBase::Area::INT_T)
									{
										auto name = std::to_string(dat[i]->ival);
										int len = name.length();
										if (len > cols[i].len) {cout << "Length Exceed." << endl; flag = 1; break;} 
										buffer.resize(cols[i].len + 1);
										memset(&buffer[0], 0, cols[i].len + 1);
										memcpy(&buffer[1], name.c_str(), len);
									}
									else {cout << "Type mismatch." << endl; flag = 1; break;}
								break;
								case hsql::kExprLiteralString:
									if (cols[i].type == SimpleDataBase::Area::VARCHAR_T)
									{
										int len = strlen(dat[i]->name);
										if (len > cols[i].len) {cout << "Length Exceed." << endl; flag = 1; break;} 
										buffer.resize(cols[i].len + 1);
										memset(&buffer[0], 0, cols[i].len + 1);
										memcpy(&buffer[1], dat[i]->name, len);
									}
									else {cout << "Type mismatch." << endl; flag = 1; break;}
								break;
								case hsql::kExprLiteralNull:
									if (cols[i].notnull) {cout << "NOTNULL Violated." << endl; flag = 1; break;}
									buffer.resize(cols[i].len + 1);
									memset(&buffer[0], 0, cols[i].len + 1);
									buffer[0] = 1;
								break;
							}
							if (flag) break;
							for (auto i: buffer) tot.push_back(i);
						}
						if (flag) break;
						ushort siz = tot.size();
						memcpy(&tot[0], &siz, 2);
						chardata.push_back(std::move(tot));
					}
					if (flag) {cout << "ERROR Occured. Insert Failed." << endl; break;}
//					for (auto const& i: chardata) for (auto const& j: i) cout << int(j) << endl;
					sys.insertRecord(tablename, chardata);
				}
				break;
			}
		} else cout << "Invalid." << endl;

		if (cin.eof()) break;
	}
	return 0;
}
