#pragma once
#include "bufmanager/BufPageManager.h"
#include "fileio/FileManager.h"
#include "utils/pagedef.h"

namespace SimpleDataBase
{

class	RecordMananger
{
public:
	RecordManager() = delete;

	inline	RecordManager(
			FileManager* fileManager
			BufPageManager* bufManager):
		fileManager(fileManager),
		bufManager(bufManager)
	{}

	bool	createFile(std::string fileName);
	bool	removeFile(std::string fileName);
	int	openFile(std::string fileName);
	int	closeFile(int fileID);

private:
	BufPageManager* bufManager;
	FileManager* fileManager;
};

}	// end namespace SimpleDataBase
