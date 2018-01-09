#SimpleDataBase

##Libraries
	Parser: https://github.com/hyrise/sql-parser

##Notice
	External parser seems a bit different from required language.

	1. Unsupported operator '`', please use direct table name instead of \`table name\`.

	2. Keyword 'USE' is parsed as 'USE DATABASE dbname' instead of 'USE dbname'.

	3. The database is stored in /data/db. Please make sure you have the right to access, or modify src/systemdb.cpp to change it into another directory.
