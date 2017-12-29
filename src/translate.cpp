#include "translate.h"
#include <cstring>
#include <iostream>

namespace	SimpleDataBase
{

std::shared_ptr<std::function<bool(void const*)> >	translate(hsql::Expr const& expr, std::map<std::string, Area> & area)
{
	if (expr.type != hsql::kExprOperator) {return nullptr;}
	switch (expr.op_type)
	{
		case hsql::Expr::NOT:
		{
			if (expr.expr->type != hsql::kExprOperator) return nullptr;
			auto sub = translate(*expr.expr, area);
			if (sub) return std::make_shared<std::function<bool(void const*)> >([sub](void const* ptr){return !(sub->operator()(ptr));});
			else return nullptr;
		}
		break;
		case hsql::Expr::AND:
		case hsql::Expr::OR:
		{
			if (expr.expr->type != hsql::kExprOperator) return nullptr;
			if (expr.expr2->type != hsql::kExprOperator) return nullptr;
			auto sub = translate(*expr.expr, area);
			if (!sub) return nullptr;
			auto sub2 = translate(*expr.expr2, area);
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
