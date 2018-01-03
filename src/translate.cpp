#include "translate.h"
#include <cstring>
#include <iostream>

using	namespace	std;

namespace	SimpleDataBase
{

std::shared_ptr<std::function<std::shared_ptr<std::string>(void const*)> >	translateString(hsql::Expr const& expr, std::map<std::string, Area> & area)
{
	auto packres = [](std::function<std::shared_ptr<std::string>(void const*)> const& fun){return std::make_shared<std::function<std::shared_ptr<std::string>(void const*)> >(fun);};
	if (expr.type == hsql::kExprColumnRef)
	{
		std::string name = expr.name;
		if (!area.count(name)) return nullptr;
		auto const& area1 = area[name];
		unsigned short o1 = area1.offset;
		unsigned short l1 = area1.len;
		if (area1.type == Area::VARCHAR_T)
		{
			return packres([o1, l1](void const* ptr) {
				char	buf[l1 + 1];
				memset(buf, 0, sizeof(buf));
				unsigned char* t = (unsigned char*) ptr;
				memcpy(buf, t + o1 + 1, l1);
				return t[o1] ? nullptr : std::make_shared<std::string>(std::string(buf));
			});
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
		return packres([tmp](void const*) {return std::make_shared<std::string>(tmp);});
	}
	else if (expr.type == hsql::kExprLiteralNull)
	{
		return packres([](void const*) {return nullptr;});
	}
	else if (expr.type == hsql::kExprOperator)
	{
		switch (expr.op_type)
		{
			case hsql::Expr::SIMPLE_OP:
			{
				auto sub = translateString(*expr.expr, area);
				if (!sub) return nullptr;
				auto sub2 = translateString(*expr.expr2, area);
				if (!sub2) return nullptr;
				if (expr.op_char == '+')
				{
					return packres([sub,sub2](void const* ptr){
						auto p1 = sub->operator()(ptr);
						auto p2 = sub2->operator()(ptr);
						return p1 && p2 ? std::make_shared<std::string>(*p1 + *p2) : nullptr;
					});
				}
			}
			break;
		}
		return nullptr;
	}
	else return nullptr;
}

std::shared_ptr<std::function<std::shared_ptr<int>(void const*)> >	translateInt(hsql::Expr const& expr, std::map<std::string, Area> & area)
{
	auto packres = [](std::function<std::shared_ptr<int>(void const*)> const& fun){return std::make_shared<std::function<std::shared_ptr<int>(void const*)> >(fun);};
	if (expr.type == hsql::kExprColumnRef)
	{
		std::string name = expr.name;
		if (!area.count(name)) return nullptr;
		auto const& area1 = area[name];
		unsigned short o1 = area1.offset;
		unsigned short l1 = area1.len;
		if (area1.type == Area::INT_T)
		{
			return packres([o1](void const* ptr) {
				int v;
				unsigned char* t = (unsigned char*) ptr;
				memcpy(&v, t + o1 + 1, 4);
				return t[o1] ? nullptr : std::make_shared<int>(v);
			});
		}
		else return nullptr;
	}
	else if (expr.type == hsql::kExprLiteralInt)
	{
		int tmp = expr.ival;
		if (tmp != expr.ival) return nullptr;
		return packres([tmp](void const*) {return std::make_shared<int>(tmp);});
	}
	else if (expr.type == hsql::kExprLiteralString)
	{
		return nullptr;
	}
	else if (expr.type == hsql::kExprLiteralNull)
	{
		return packres([](void const*) {return nullptr;});
	}
	else if (expr.type == hsql::kExprOperator)
	{
		switch (expr.op_type)
		{
			case hsql::Expr::SIMPLE_OP:
			{
				auto sub = translateInt(*expr.expr, area);
				if (!sub) return nullptr;
				auto sub2 = translateInt(*expr.expr2, area);
				if (!sub2) return nullptr;
				if (expr.op_char == '+')
				{
					return packres([sub,sub2](void const* ptr){
						auto p1 = sub->operator()(ptr);
						auto p2 = sub2->operator()(ptr);
						return p1 && p2 ? std::make_shared<int>(*p1 + *p2) : nullptr;
					});
				}
				if (expr.op_char == '-')
				{
					return packres([sub,sub2](void const* ptr){
						auto p1 = sub->operator()(ptr);
						auto p2 = sub2->operator()(ptr);
						return p1 && p2 ? std::make_shared<int>(*p1 - *p2) : nullptr;
					});
				}
				if (expr.op_char == '*')
				{
					return packres([sub,sub2](void const* ptr){
						auto p1 = sub->operator()(ptr);
						auto p2 = sub2->operator()(ptr);
						return p1 && p2 ? std::make_shared<int>(*p1 * *p2) : nullptr;
					});
				}
				if (expr.op_char == '/')
				{
					return packres([sub,sub2](void const* ptr){
						auto p1 = sub->operator()(ptr);
						auto p2 = sub2->operator()(ptr);
						return p1 && p2 && (*p2 != 0) ? std::make_shared<int>(*p1 + *p2) : nullptr;
					});
				}
			}
			break;
		}
		return nullptr;
	}
	else return nullptr;
}

std::shared_ptr<std::function<bool(void const*)> >	translateCond(hsql::Expr const& expr, std::map<std::string, Area> & area)
{
	if (expr.type != hsql::kExprOperator) {return nullptr;}
	switch (expr.op_type)
	{
		case hsql::Expr::NOT:
		{
			if (expr.expr->type != hsql::kExprOperator) return nullptr;
			auto sub = translateCond(*expr.expr, area);
			if (sub) return std::make_shared<std::function<bool(void const*)> >([sub](void const* ptr){return !(sub->operator()(ptr));});
			else return nullptr;
		}
		break;
		case hsql::Expr::AND:
		case hsql::Expr::OR:
		{
			auto sub = translateCond(*expr.expr, area);
			if (!sub) return nullptr;
			auto sub2 = translateCond(*expr.expr2, area);
			if (!sub2) return nullptr;
			if (expr.op_type == hsql::Expr::AND) return std::make_shared<std::function<bool(void const*)> >([sub, sub2](void const* ptr){return sub->operator()(ptr) && sub2->operator()(ptr);});
			else return std::make_shared<std::function<bool(void const*)> >([sub, sub2](void const* ptr){return sub->operator()(ptr) || sub2->operator()(ptr);});
		}
		break;
		case hsql::Expr::SIMPLE_OP:
		case hsql::Expr::NOT_EQUALS:
		case hsql::Expr::LESS_EQ:
		case hsql::Expr::GREATER_EQ:
		{
			if (!(expr.expr || expr.expr2)) return nullptr;
			auto pli = translateInt(*expr.expr, area);
			auto pri = translateInt(*expr.expr2, area);
			if (pli && pri)
			{
				auto li = *pli, ri = *pri;
				if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '=')
				{
					return std::make_shared<std::function<bool(void const*)> >([li, ri](void const* ptr) {
						auto x = li(ptr);
						auto y = ri(ptr);
						return x && y ? *x == *y : false;
					});
				}
				if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '<')
				{
					return std::make_shared<std::function<bool(void const*)> >([li, ri](void const* ptr) {
						auto x = li(ptr);
						auto y = ri(ptr);
						return x && y ? *x < *y : false;
					});
				}
				if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '>')
				{
					return std::make_shared<std::function<bool(void const*)> >([li, ri](void const* ptr) {
						auto x = li(ptr);
						auto y = ri(ptr);
						return x && y ? *x > *y : false;
					});
				}
				if (expr.op_type == hsql::Expr::NOT_EQUALS)
				{
					return std::make_shared<std::function<bool(void const*)> >([li, ri](void const* ptr) {
						auto x = li(ptr);
						auto y = ri(ptr);
						return x && y ? *x != *y : false;
					});
				}
				if (expr.op_type == hsql::Expr::LESS_EQ)
				{
					return std::make_shared<std::function<bool(void const*)> >([li, ri](void const* ptr) {
						auto x = li(ptr);
						auto y = ri(ptr);
						return x && y ? *x <= *y : false;
					});
				}
				if (expr.op_type == hsql::Expr::GREATER_EQ)
				{
					return std::make_shared<std::function<bool(void const*)> >([li, ri](void const* ptr) {
						auto x = li(ptr);
						auto y = ri(ptr);
						return x && y ? *x >= *y : false;
					});
				}
			}
			auto pls = translateString(*expr.expr, area);
			auto prs = translateString(*expr.expr2, area);
			if (pls && prs)
			{
				auto ls = *pls;
				auto rs = *prs;
				if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '=')
				{
					return std::make_shared<std::function<bool(void const*)> >([ls, rs](void const* ptr) {
						auto x = ls(ptr);
						auto y = rs(ptr);
						return x && y ? *x == *y : false;
					});
				}
				if (expr.op_type == hsql::Expr::NOT_EQUALS)
				{
					return std::make_shared<std::function<bool(void const*)> >([ls, rs](void const* ptr) {
						auto x = ls(ptr);
						auto y = rs(ptr);
						return x && y ? *x != *y : false;
					});
				}
			}
			return nullptr;
		}
		break;
	}
	return nullptr;
}

}	// end namespace SimpleDataBase
