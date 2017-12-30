#include "translate.h"
#include <cstring>
#include <iostream>

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
			if (expr.expr->type == hsql::kExprColumnRef && expr.expr2->type == hsql::kExprColumnRef)
				return nullptr;
			if (expr.expr->type != hsql::kExprColumnRef && expr.expr2->type != hsql::kExprColumnRef)
				return nullptr;
			auto p = expr.expr, q = expr.expr2;
			if (expr.expr->type != hsql::kExprColumnRef) p = expr.expr2, q = expr.expr;
			std::string name = p->name;
			if (!area.count(name)) return nullptr;
			auto const& area1 = area[name];
			unsigned short o1 = area1.offset;
			unsigned short l1 = area1.len;
			if (q->type == hsql::kExprLiteralNull)
			{
				return std::make_shared<std::function<bool(void const*)> >([](void const* ptr) {
					return false;
				});
			}
			if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '=')
			{
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralInt)
				{
					int tmp = q->ival;
					if (tmp != q->ival) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v == tmp;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralString)
				{
					std::string tmp = q->name;
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) == 0;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralInt)
				{
					std::string tmp = std::to_string(q->ival);
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) == 0;
					});
				}
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralString)
				{
					int tmp = std::stoi(q->name);
					if (std::to_string(tmp) != q->name) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v == tmp;
					});
				}
			}
			if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '<')
			{
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralInt)
				{
					int tmp = q->ival;
					if (tmp != q->ival) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v < tmp;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralString)
				{
					std::string tmp = q->name;
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) < 0;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralInt)
				{
					std::string tmp = std::to_string(q->ival);
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) < 0;
					});
				}
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralString)
				{
					int tmp = std::stoi(q->name);
					if (std::to_string(tmp) != q->name) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v < tmp;
					});
				}
			}
			if (expr.op_type == hsql::Expr::SIMPLE_OP && expr.op_char == '>')
			{
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralInt)
				{
					int tmp = q->ival;
					if (tmp != q->ival) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v > tmp;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralString)
				{
					std::string tmp = q->name;
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) > 0;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralInt)
				{
					std::string tmp = std::to_string(q->ival);
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) > 0;
					});
				}
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralString)
				{
					int tmp = std::stoi(q->name);
					if (std::to_string(tmp) != q->name) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v > tmp;
					});
				}
			}
			if (expr.op_type == hsql::Expr::NOT_EQUALS)
			{
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralInt)
				{
					int tmp = q->ival;
					if (tmp != q->ival) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v != tmp;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralString)
				{
					std::string tmp = q->name;
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) != 0;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralInt)
				{
					std::string tmp = std::to_string(q->ival);
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) != 0;
					});
				}
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralString)
				{
					int tmp = std::stoi(q->name);
					if (std::to_string(tmp) != q->name) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v != tmp;
					});
				}
			}
			if (expr.op_type == hsql::Expr::LESS_EQ)
			{
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralInt)
				{
					int tmp = q->ival;
					if (tmp != q->ival) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v <= tmp;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralString)
				{
					std::string tmp = q->name;
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) <= 0;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralInt)
				{
					std::string tmp = std::to_string(q->ival);
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) <= 0;
					});
				}
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralString)
				{
					int tmp = std::stoi(q->name);
					if (std::to_string(tmp) != q->name) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v <= tmp;
					});
				}
			}
			if (expr.op_type == hsql::Expr::GREATER_EQ)
			{
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralInt)
				{
					int tmp = q->ival;
					if (tmp != q->ival) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v >= tmp;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralString)
				{
					std::string tmp = q->name;
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) >= 0;
					});
				}
				if (area1.type == Area::VARCHAR_T && q->type == hsql::kExprLiteralInt)
				{
					std::string tmp = std::to_string(q->ival);
					if (l1 < tmp.size()) l1 = tmp.size();
					return std::make_shared<std::function<bool(void const*)> >([o1, l1, tmp](void const* ptr) {
						char buf1[l1], buf2[l1];
						unsigned char* t = (unsigned char*) ptr;
						memset(buf1, 0, sizeof(buf1));
						memset(buf2, 0, sizeof(buf2));
						memcpy(buf1, t + o1 + 1, sizeof(buf1));
						memcpy(buf2, tmp.c_str(), tmp.length());
						return (!t[o1]) && memcmp(buf1, buf2, l1) >= 0;
					});
				}
				if (area1.type == Area::INT_T && q->type == hsql::kExprLiteralString)
				{
					int tmp = std::stoi(q->name);
					if (std::to_string(tmp) != q->name) return nullptr;
					return std::make_shared<std::function<bool(void const*)> >([o1, tmp](void const* ptr) {
						int v;
						unsigned char* t = (unsigned char*) ptr;
						memcpy(&v, t + o1 + 1, 4);
						return (!t[o1]) && v >= tmp;
					});
				}
			}
		}
		break;
	}
	return nullptr;
}

}	// end namespace SimpleDataBase
