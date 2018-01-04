#pragma once

#include "SQLParser.h"
#include "area.h"

#include <map>
#include <vector>
#include <string>
#include <functional>

namespace	SimpleDataBase
{

std::shared_ptr <int>	checkInt(hsql::Expr const& expr, std::map <std::string, std::map<std::string, Area> > &area,
		std::map < std::string, char* > & obj);

std::shared_ptr <std::string>	checkString(hsql::Expr const& expr, std::map <std::string, std::map<std::string, Area> > &area,
		std::map < std::string, char* > & obj);

int	checkCond(hsql::Expr const& expr, std::map <std::string, std::map<std::string, Area> > & area, 
		std::map < std::string, char* > & obj);

inline	int	checkCond(hsql::Expr const& expr, std::vector <Area> const & area, 
		std::map < std::string, char* > & obj)
{
	std::map < std::string, std::map< std::string, Area > > area_lookup;
	for (auto const& a: area) area_lookup[a.table][a.name] = a;
	return checkCond(expr, area_lookup, obj);
}

void	checkIterate(hsql::Expr const* expr, std::map <std::string, std::vector < std::vector < char > > > bases,
		std::map < std::string, std::map < std::string, Area > > &area,
		std::function<void(std::map < std::string, char* >&)> const& op);

}	// end namespace SimpleDataBase
