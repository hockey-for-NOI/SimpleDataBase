#include "check.h"
#include <cstring>

namespace	SimpleDataBase
{

int	checkCond(hsql::Expr const& expr, std::map <std::string, std::map<std::string, Area> > & area, 
		std::map < std::string, std::vector <char> > & obj)
{

	if (expr.type != hsql::kExprOperator) {return -1;}
	switch (expr.op_type)
	{
		case hsql::Expr::NOT:
		{
			int tmp = checkCond(*expr.expr, area, obj);
			return tmp == -1 ? -1 : !tmp;
		}
		break;
		case hsql::Expr::AND:
		{
			int tmp = checkCond(*expr.expr, area, obj);
			if (tmp == -1) return -1;
			int tmp2 = checkCond(*expr.expr2, area, obj);
			if (tmp2 == -1) return -1;
			return tmp && tmp2;
		}
		break;
		case hsql::Expr::OR:
		{
			int tmp = checkCond(*expr.expr, area, obj);
			if (tmp == -1) return -1;
			int tmp2 = checkCond(*expr.expr2, area, obj);
			if (tmp2 == -1) return -1;
			return tmp || tmp2;
		}
		break;
		case hsql::Expr::SIMPLE_OP:
		case hsql::Expr::NOT_EQUALS:
		case hsql::Expr::LESS_EQ:
		case hsql::Expr::GREATER_EQ:
		{
			unsigned short lt, rt;
			int li, ri;
			std::string ls, rs;
			if (expr.expr->type == hsql::kExprColumnRef)
			{
				std::string table = expr.expr->table;
				std::string name = expr.expr->name;
				if (!area.count(table)) return -1;
				if (!area[table].count(name)) return -1;
				if (!obj.count(table)) return -1;
				auto const& area1 = area[table][name];
				auto const& obj1 = obj[table];
				if (obj1[area1.offset]) lt = Area::RESERVED_T;
				else if (area1.type == Area::INT_T)
				{
					lt = Area::INT_T;
					memcpy(&li, &obj1[area1.offset + 1], 4);
				}
				else if (area1.type == Area::VARCHAR_T)
				{
					char buf[area1.len + 1]; buf[area1.len] = 0;
					lt = Area::VARCHAR_T;
					memcpy(buf, &obj1[area1.offset + 1], area1.len);
					ls = buf;
				}
				else return -1;
			}
			else if (expr.expr->type == hsql::kExprLiteralInt)
			{
				lt = Area::INT_T;
				li = expr.expr->ival;
			}
			else if (expr.expr->type == hsql::kExprLiteralString)
			{
				lt = Area::VARCHAR_T;
				ls = expr.expr->name;
			}
			else if (expr.expr->type == hsql::kExprLiteralNull)
			{
				lt = Area::RESERVED_T;
			}

			if (expr.expr2->type == hsql::kExprColumnRef)
			{
				std::string table = expr.expr2->table;
				std::string name = expr.expr2->name;
				if (!area.count(table)) return -1;
				if (!area[table].count(name)) return -1;
				if (!obj.count(table)) return -1;
				auto const& area1 = area[table][name];
				auto const& obj1 = obj[table];
				if (obj1[area1.offset]) rt = Area::RESERVED_T;
				else if (area1.type == Area::INT_T)
				{
					rt = Area::INT_T;
					memcpy(&ri, &obj1[area1.offset + 1], 4);
				}
				else if (area1.type == Area::VARCHAR_T)
				{
					char buf[area1.len + 1]; buf[area1.len] = 0;
					rt = Area::VARCHAR_T;
					memcpy(buf, &obj1[area1.offset + 1], area1.len);
					rs = buf;
				}
				else return -1;
			}
			else if (expr.expr2->type == hsql::kExprLiteralInt)
			{
				rt = Area::INT_T;
				ri = expr.expr2->ival;
			}
			else if (expr.expr2->type == hsql::kExprLiteralString)
			{
				rt = Area::VARCHAR_T;
				rs = expr.expr2->name;
			}
			else if (expr.expr2->type == hsql::kExprLiteralNull)
			{
				rt = Area::RESERVED_T;
			}

			if (lt == Area::RESERVED_T || rt == Area::RESERVED_T) return -1;
			if (lt == Area::INT_T && rt == Area::VARCHAR_T)
			{
				ri = std::stoi(rs);
				if (std::to_string(ri) != rs) return -1;
				rt = Area::INT_T;
			}
			if (rt == Area::INT_T && lt == Area::VARCHAR_T)
			{
				li = std::stoi(ls);
				if (std::to_string(li) != ls) return -1;
				lt = Area::INT_T;
			}

			if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '=')
			{
				return lt == Area::INT_T ? li == ri : ls == rs;
			}
			if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '<')
			{
				return lt == Area::INT_T ? li < ri : ls < rs;
			}
			if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '>')
			{
				return lt == Area::INT_T ? li > ri : ls > rs;
			}
			if (expr.op_type == hsql::Expr::NOT_EQUALS)
			{
				return lt == Area::INT_T ? li != ri : ls != rs;
			}
			if (expr.op_type == hsql::Expr::LESS_EQ)
			{
				return lt == Area::INT_T ? li <= ri : ls <= rs;
			}
			if (expr.op_type == hsql::Expr::GREATER_EQ)
			{
				return lt == Area::INT_T ? li >= ri : ls >= rs;
			}
			return -1;
		}
		break;
	}
}

}	// end namespace SimpleDataBase
