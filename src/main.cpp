#include <iostream>
#include <string>
#include <cstring>
#include "systemdb.h"
#include "translate.h"
#include "check.h"

#include "SQLParser.h"
#include <map>
#include <iomanip>

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
										area.len = 4;
										area.pad = cols[i]->len;
										area.type = SimpleDataBase::Area::INT_T;
									break;
									case hsql::ColumnDefinition::VARCHAR:
										area.len = cols[i]->len;
										area.pad = cols[i]->len;
										area.type = SimpleDataBase::Area::VARCHAR_T;
									break;
								}
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
										int tmp = dat[i]->ival;
										if (tmp != dat[i]->ival) {cout << "Integer Exceed." << endl; flag = 1; break;} 
										buffer.resize(5);
										memcpy(&buffer[1], &tmp, 4);
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
					if (!sys.insertRecord(tablename, chardata)) {cout << "ERROR Occured. Insert Failed." << endl; break;}
				}
				break;
				case hsql::kStmtDelete:
				{
					auto &sys = SimpleDataBase::SystemDB::get_instance();
					if (!sys.getCurrentDB().length()) {cout << "ERROR: No current DB." << endl; break;}
					auto dstmt = dynamic_cast<hsql::DeleteStatement*>(stmt);
					auto tablename = std::string(dstmt->tableName);
					if (!sys.hasTable(tablename)) {cout << "ERROR: No such table." << endl; break;}
					auto cols = sys.getTableCols(tablename);
					auto tr = SimpleDataBase::translateCond(dstmt->expr, cols);
					if (!tr) {cout << "Condition ERROR." << endl; break;}
					cout << sys.deleteRecord(tablename, *tr) << " Item(s) Deleted." << endl;
				}
				break;
				case hsql::kStmtSelect:
				{
					auto &sys = SimpleDataBase::SystemDB::get_instance();
					if (!sys.getCurrentDB().length()) {cout << "ERROR: No current DB." << endl; break;}
					auto sstmt = dynamic_cast<hsql::SelectStatement*>(stmt);
					if (sstmt->fromTable->type == hsql::kTableName)
					{
						auto tablename = std::string(sstmt->fromTable->name);
						if (!sys.hasTable(tablename)) {cout << "ERROR: No such table." << endl; break;}
						auto cols = sys.getTableCols(tablename);
						std::map < std::string, SimpleDataBase::Area > tmp;
						for (auto const& a: cols) tmp[std::string(a.name)] = a;
						auto tr = SimpleDataBase::translateCond(sstmt->whereClause, cols);
						if (!tr) {cout << "Condition ERROR." << endl; break;}
						bool flag = 0;
						std::vector <SimpleDataBase::Area> toselect;
						for (auto field: *sstmt->selectList)
						{
							switch (field->type)
							{
								case hsql::kExprStar:
									toselect = cols;
								break;
								case hsql::kExprColumnRef:
									if (!tmp.count(std::string(field->name))) {flag = 1; break;}
									toselect.push_back(tmp[std::string(field->name)]);
								break;
							}
						}
						if (flag) {cout << "Select Field ERROR." << endl; break;}
						auto res = sys.selectRecord(tablename, *tr); 
						for (auto const& obj: res)
						{
							for (auto const& pr: toselect)
							{
								if (pr.type == SimpleDataBase::Area::INT_T)
								{
									if (obj[pr.offset]) {cout << setw(pr.pad) << "NULL"; continue;}
									int	x;
									memcpy(&x, &obj[pr.offset + 1], 4);
									cout << setw(pr.pad) << x;
								}
								else
								{
									if (obj[pr.offset]) {cout << "NULL"; continue;}
									char buf[pr.len + 1];
									memset(buf, 0, sizeof(buf));
									memcpy(&buf, &obj[pr.offset + 1], pr.len);
									cout << buf;
								}
							}
							cout << endl;
						}
					}
					else if (sstmt->fromTable->type == hsql::kTableCrossProduct)
					{
						std::vector <SimpleDataBase::Area> cols;
						std::map < std::string, std::vector < std::vector <char> > > bases;
						std::map < std::string, std::map < std::string, SimpleDataBase::Area > > tmp;
						for (auto i: *sstmt->fromTable->list)
						{
							std::string tname = i->name;
							auto tcols = sys.getTableCols(tname);
							for (auto const& t: tcols) cols.push_back(t);
							bases[tname] = sys.selectRecord(tname, [](void const*) {return 1;});
						}
						for (auto const& i: cols) tmp[i.table][i.name] = i;
						std::vector <SimpleDataBase::Area> toselect;
						bool flag = 0;
						for (auto field: *sstmt->selectList)
						{
							switch (field->type)
							{
								case hsql::kExprStar:
									toselect = cols;
								break;
								case hsql::kExprColumnRef:
									if (!tmp.count(field->table)) {flag = 1; break;}
									if (!tmp[field->table].count(field->name)) {flag = 1; break;}
									toselect.push_back(tmp[field->table][field->name]);
								break;
							}
						}
						if (flag) {cout << "Select Field ERROR." << endl; break;}
						auto disp = [toselect](std::map<std::string, std::vector<char> > &obj){
							for (auto const& pr: toselect)
							{
								auto const& obj1 = obj[pr.table];
								if (pr.type == SimpleDataBase::Area::INT_T)
								{
									if (obj1[pr.offset]) {cout << setw(pr.pad) << "NULL"; continue;}
									int	x;
									memcpy(&x, &obj1[pr.offset + 1], 4);
									cout << setw(pr.pad) << x;
								}
								else
								{
									if (obj1[pr.offset]) {cout << "NULL"; continue;}
									char buf[pr.len + 1];
									memset(buf, 0, sizeof(buf));
									memcpy(&buf, &obj1[pr.offset + 1], pr.len);
									cout << buf;
								}
							}
							cout << endl;
						};
						checkIterate(sstmt->whereClause, bases, tmp, disp);
					}
					else cout << "Unimplemented select type." << endl;
				}
				break;
				case hsql::kStmtUpdate:
				{
					auto &sys = SimpleDataBase::SystemDB::get_instance();
					if (!sys.getCurrentDB().length()) {cout << "ERROR: No current DB." << endl; break;}
					auto ustmt = dynamic_cast<hsql::UpdateStatement*>(stmt);
					auto tablename = std::string(ustmt->table->name);
					if (!sys.hasTable(tablename)) {cout << "ERROR: No such table." << endl; break;}
					auto cols = sys.getTableCols(tablename);
					auto tr = SimpleDataBase::translateCond(ustmt->where, cols);
					if (!tr) {cout << "Condition ERROR." << endl; break;}
					auto &upd = *ustmt->updates;
					std::vector < std::function< bool(void*) > > cvtupd;
					std::map < std::string, SimpleDataBase::Area > tmp;
					for (auto const& a: cols) tmp[std::string(a.name)] = a;
					bool flag = 0;
					for (auto updexpr: upd)
					{
						if (!tmp.count(std::string(updexpr->column))) {cout << "ERROR: No column " << updexpr->column << "." << endl; flag = 1; break;}
						auto &col = tmp[std::string(updexpr->column)];
						if (col.type == SimpleDataBase::Area::INT_T)
						{
							auto tr = SimpleDataBase::translateInt(*updexpr->value, cols);
							if (!tr) {cout << "ERROR: Translation failed in expr for update column " << updexpr->column << "." << endl; flag = 1; break;}
							unsigned short o1 = col.offset;
							cvtupd.push_back([o1, tr](void *ptr){
								char* t = (char*)ptr;
								auto grab = tr->operator()(ptr);
								if (grab)
								{
									t[o1] = 0;
									memcpy(t + o1 + 1, grab.get(), 4);
									return 1;
								}
								else {t[o1] = 1; return 1;}
							});
						}
						else if (col.type == SimpleDataBase::Area::VARCHAR_T)
						{
							auto tr = SimpleDataBase::translateString(*updexpr->value, cols);
							if (!tr) {cout << "ERROR: Translation failed in expr for update column " << updexpr->column << "." << endl; flag = 1; break;}
							unsigned short o1 = col.offset;
							unsigned short l1 = col.len;
							cvtupd.push_back([o1, l1, tr](void *ptr){
								char* t = (char*)ptr;
								auto grab = tr->operator()(ptr);
								if (grab)
								{
									if (grab->length() <= l1)
									{
										char buf[l1 + 1];
										memset(buf, 0, sizeof(buf));
										strcpy(buf, grab->c_str());
										t[o1] = 0;
										memcpy(t + o1 + 1, buf, l1);
										return 1;
									}
									else return 0;
								}
								else
								{
									t[o1] = 1;
									return 1;
								}
							});
						}
					}
					if (flag) {cout << "ERROR Occured. Update Failed." << endl; break;}
					auto res = sys.updateRecord(tablename, *tr, [cvtupd](void const* ptr){
						std::vector <char> data;
						ushort totlen;
						memcpy(&totlen, ptr, 2);
						data.resize(totlen);
						memcpy(&data[0], ptr, totlen);

						//for (auto i: data) cout << int(i) << endl;
						
						bool flag = 0;
						for (auto const& upd: cvtupd) if (!upd(&data[0])) {flag = 1; break;};

						//for (auto i: data) cout << int(i) << endl;

						if (flag) return std::vector<char>(); else return data;
					});
					cout << res.first << " out of " << res.first + res.second << " update succeeded." << endl;
				}
				break;
			}
		} else cout << "Invalid." << endl;

		if (cin.eof()) break;
	}
	return 0;
}
