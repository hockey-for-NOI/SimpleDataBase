#include <iostream>
#include <string>
#include "systemdb.h"

#include "external/hsql/SQLParser.h"

using	namespace	std;

int	main()
{
	cout << "Loading..."; cout.flush();
	auto	&sys = SimpleDataBase::SystemDB::get_instance();
	cout << "Done." << endl;

	while (true)
	{
		cout << "[" << sys.getCurrentDB() << "]>"; cout.flush();
		std::string input_str;
		getline(cin, input_str);
		auto result = hsql::SQLParser::parseSQLString(input_str);

		if (result->isValid()) for (int i=0; i<result->size(); i++)
		{
			
		}

		if (cin.eof()) break;
	}
	return 0;
}
