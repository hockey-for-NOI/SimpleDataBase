#pragma once

#include "SQLParser.h"
#include "area.h"

#include <memory>
#include <map>
#include <vector>
#include <string>

namespace	SimpleDataBase
{

std::shared_ptr<std::function<bool(void const*)> >	translateCond(hsql::Expr const& expr, std::map<std::string, Area> & area);

inline	std::shared_ptr<std::function<bool(void const*)> >	translateCond(hsql::Expr const& expr, std::vector<Area> const& area)
{
	std::map<std::string, Area> tmp;
	for (auto const& a: area) tmp[std::string(a.name)] = a;
	return translateCond(expr, tmp);
}

std::shared_ptr<std::function<std::shared_ptr<int>(void const*)> >	translateInt(hsql::Expr const& expr, std::map<std::string, Area> & area);

inline	std::shared_ptr<std::function<std::shared_ptr<int>(void const*)> >	translateInt(hsql::Expr const& expr, std::vector<Area> const& area)
{
	std::map<std::string, Area> tmp;
	for (auto const& a: area) tmp[std::string(a.name)] = a;
	return translateInt(expr, tmp);
}

std::shared_ptr<std::function<std::shared_ptr<std::string>(void const*)> >	translateString(hsql::Expr const& expr, std::map<std::string, Area> & area);

inline	std::shared_ptr<std::function<std::shared_ptr<std::string>(void const*)> >	translateString(hsql::Expr const& expr, std::vector<Area> const& area)
{
	std::map<std::string, Area> tmp;
	for (auto const& a: area) tmp[std::string(a.name)] = a;
	return translateString(expr, tmp);
}

}	// end namespace SimpleDataBase
