#pragma once

#include "SQLParser.h"
#include "area.h"

#include <memory>
#include <map>
#include <vector>
#include <string>

namespace	SimpleDataBase
{

std::shared_ptr<std::function<bool(void const*)> >	translate(hsql::Expr const& expr, std::map<std::string, Area> & area);

inline	std::shared_ptr<std::function<bool(void const*)> >	translate(hsql::Expr const& expr, std::vector<Area> const& area)
{
	std::map<std::string, Area> tmp;
	for (auto const& a: area) tmp[std::string(a.name)] = a;
	return translate(expr, tmp);
}

}	// end namespace SimpleDataBase
