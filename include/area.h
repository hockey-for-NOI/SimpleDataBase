#pragma once
#include <string>

namespace	SimpleDataBase
{

struct	Area
{
	typedef	unsigned	short ushort;
	ushort	size;
	enum ushort {INT_T, VARCHAR_T, FLOAT_T, DATE_T} type;
	ushort	offset, len;
	char	name[256], table[256];

	std::string	showtype();
};

}	// end namespace SimpleDataBase
