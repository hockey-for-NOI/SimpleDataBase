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

		if (result->isValid) for (int i=0; i<result->size(); i++)
		{
			auto const& stmt = result->statements[i];
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
							}
							std::vector <SimpleDataBase::Area> areas_vec;
							unsigned short offset = 2;
							for (auto const& i: areas)
							{
								areas_vec.push_back(i.second);
								areas_vec.back().offset = offset;
								offset += areas_vec.back().len + 1;
							}
							SimpleDataBase::SystemDB::get_instance().createTable(cstmt->tableName, areas_vec);
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
							SimpleDataBase::SystemDB::get_instance().dropDB(dstmt->name);
						break;
						case hsql::DropStatement::kTable:
							SimpleDataBase::SystemDB::get_instance().dropTable(dstmt->name);
						break;
					}
				}
				break;
				case hsql::kStmtUse:
				{
					auto ustmt = dynamic_cast<hsql::UseStatement*>(stmt);
					SimpleDataBase::SystemDB::get_instance().useDB(ustmt->name);
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
			}
		} else cout << "Invalid." << endl;

		if (cin.eof()) break;
	}
	return 0;
}
