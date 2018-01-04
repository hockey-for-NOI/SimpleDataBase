#include "check.h"

namespace	SimpleDataBase
{

void	checkIterate(hsql::Expr const* expr, std::map <std::string, std::vector < std::vector < char > > > bases,
		std::map < std::string, std::map < std::string, Area > > &area,
		std::function<void(std::map < std::string, char* >&)> const& op)
{
	std::vector <std::string> names;
	std::vector <int> sizes, nowid;
	int	depth;
	std::map <std::string, char* > now;

	for (auto const& i: bases) names.push_back(i.first), sizes.push_back(i.second.size());
	nowid.resize(sizes.size());
	depth = 0;

	while (depth >= 0)
	{
		auto const& name = names[depth];
		if (depth == sizes.size())
		{
			if (!expr || checkCond(*expr, area, now) == 1) op(now);
			--depth;
			continue;
		}
		if (nowid[depth] == sizes[depth]) --depth;
		else
		{
			now[name] = &bases[name][nowid[depth++]++][0];
			nowid[depth] = 0;
		}
	}
}

}	// end namespace SimpleDataBase
