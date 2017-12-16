#include "area.h"

namespace	SimpleDataBase
{

std::string	Area::showtype() const
{
	switch (type)
	{
		case INT_T:
			return "INT(" + std::to_string(len) + ")";
		break;
		case VARCHAR_T:
			return "VARCHAR(" + std::to_string(len) + ")";
		break;
		case DATE_T:
			return "DATE";
		break;
		case FLOAT_T:
			return "FLOAT";
		break;
	}
}

}	// end namespace SimpleDataBase
