#include "check.h"
#include <cstring>
#include <iostream>

using	namespace	std;

namespace	SimpleDataBase
{

std::shared_ptr <std::string> checkString(hsql::Expr const& expr, std::map <std::string, std::map<std::string, Area> > & area, 
		std::map < std::string, std::vector <char> > & obj)
{
	if (expr.type == hsql::kExprColumnRef)
	{
		std::string table = expr.table;
		if (!area.count(table)) return nullptr;
		std::string name = expr.name;
		if (!area[table].count(name)) return nullptr;
		auto const& area1 = area[table][name];
		unsigned short o1 = area1.offset;
		unsigned short l1 = area1.len;
		if (area1.type == Area::VARCHAR_T)
		{
			char	buf[l1 + 1];
			memset(buf, 0, sizeof(buf));
			char* t = &obj[table][0];
			memcpy(buf, t + o1 + 1, l1);
			return t[o1] ? nullptr : std::make_shared<std::string>(std::string(buf));
		}
		else return nullptr;
	}
	else if (expr.type == hsql::kExprLiteralInt)
	{
		return nullptr;
	}
	else if (expr.type == hsql::kExprLiteralString)
	{
		std::string tmp = expr.name;
		return std::make_shared<std::string>(tmp);
	}
	else if (expr.type == hsql::kExprLiteralNull)
	{
		return nullptr;
	}
	else if (expr.type == hsql::kExprOperator)
	{
		switch (expr.op_type)
		{
			case hsql::Expr::SIMPLE_OP:
			{
				auto sub = checkString(*expr.expr, area, obj);
				if (!sub) return nullptr;
				auto sub2 = checkString(*expr.expr2, area, obj);
				if (!sub2) return nullptr;
				if (expr.op_char == '+')
				{
					return (sub && sub2) ? std::make_shared<std::string>((*sub) + (*sub2)) : nullptr;
				}
			}
			break;
		}
		return nullptr;
	}
	else return nullptr;
}

std::shared_ptr <int> checkInt(hsql::Expr const& expr, std::map <std::string, std::map<std::string, Area> > & area, 
		std::map < std::string, std::vector <char> > & obj)
{
	if (expr.type == hsql::kExprColumnRef)
	{
		std::string table = expr.table;
		if (!area.count(table)) return nullptr;
		std::string name = expr.name;
		if (!area[table].count(name)) return nullptr;
		auto const& area1 = area[table][name];
		unsigned short o1 = area1.offset;
		unsigned short l1 = area1.len;
		if (area1.type == Area::INT_T)
		{
			int v;
			char* t = &obj[table][0];
			memcpy(&v, t + o1 + 1, 4);
			return t[o1] ? nullptr : std::make_shared<int>(v);
		}
		else return nullptr;
	}
	else if (expr.type == hsql::kExprLiteralInt)
	{
		int tmp = expr.ival;
		if (tmp != expr.ival) return nullptr;
		return std::make_shared<int>(tmp);
	}
	else if (expr.type == hsql::kExprLiteralString)
	{
		return nullptr;
	}
	else if (expr.type == hsql::kExprLiteralNull)
	{
		return nullptr;
	}
	else if (expr.type == hsql::kExprOperator)
	{
		switch (expr.op_type)
		{
			case hsql::Expr::SIMPLE_OP:
			{
				auto sub = checkInt(*expr.expr, area, obj);
				if (!sub) return nullptr;
				auto sub2 = checkInt(*expr.expr2, area, obj);
				if (!sub2) return nullptr;
				if (expr.op_char == '+')
				{
					return (sub && sub2) ? std::make_shared<int>((*sub) + (*sub2)) : nullptr;
				}
				if (expr.op_char == '-')
				{
					return (sub && sub2) ? std::make_shared<int>((*sub) - (*sub2)) : nullptr;
				}
				if (expr.op_char == '*')
				{
					return (sub && sub2) ? std::make_shared<int>((*sub) * (*sub2)) : nullptr;
				}
				if (expr.op_char == '/')
				{
					return (sub && sub2 && (*sub2)) ? std::make_shared<int>((*sub) / (*sub2)) : nullptr;
				}
			}
			break;
		}
		return nullptr;
	}
	else return nullptr;
}

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
			lt = Area::RESERVED_T;
			auto ti = checkInt(*expr.expr, area, obj);
			if (ti) lt = Area::INT_T, li = *ti;
			auto ts = checkString(*expr.expr, area, obj);
			if (ts) lt = Area::VARCHAR_T, ls = *ts;
			rt = Area::RESERVED_T;
			ti = checkInt(*expr.expr2, area, obj);
			if (ti) rt = Area::INT_T, ri = *ti;
			ts = checkString(*expr.expr2, area, obj);
			if (ts) rt = Area::VARCHAR_T, rs = *ts;

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
				return lt == Area::INT_T ? li < ri : -1;
			}
			if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '>')
			{
				return lt == Area::INT_T ? li > ri : -1;
			}
			if (expr.op_type == hsql::Expr::NOT_EQUALS)
			{
				return lt == Area::INT_T ? li != ri : ls != rs;
			}
			if (expr.op_type == hsql::Expr::LESS_EQ)
			{
				return lt == Area::INT_T ? li <= ri : -1;
			}
			if (expr.op_type == hsql::Expr::GREATER_EQ)
			{
				return lt == Area::INT_T ? li >= ri : -1;
			}
			return -1;
		}
		break;
	}
}

}	// end namespace SimpleDataBase
