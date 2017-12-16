#pragma once
#include <string>

namespace	SimpleDataBase
{

struct	Area
{
	static	const	int	INT_T = 1;
	static	const	int	VARCHAR_T = 2;
	static	const	int FLOAT_T = 3;
	static	const	int	DATE_T = 4;

	typedef	unsigned	short ushort;
	ushort	size;
	ushort	type; //{INT_T, VARCHAR_T, FLOAT_T, DATE_T}
	ushort	offset, len;
	char	name[256], table[256];

	std::string	showtype() const;
};

}	// end namespace SimpleDataBase
