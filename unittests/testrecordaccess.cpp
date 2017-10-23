#include "recordman.h"
#include <cstdio>
#include <cstring>

using namespace SimpleDataBase;

struct	RecordA
{
	uchar data[1000];
	RecordA() {memset(data, 'A', sizeof(data));}
};

struct	RecordB
{
	uchar data[500];
	RecordB() {memset(data, 'B', sizeof(data));}
};

struct	RecordC
{
	uchar data[8190];
	RecordC() {memset(data, 'C', sizeof(data));}
};

int	main()
{
	RecordManager rm;
	rm.createFile("test1.dat");
	rm.createFile("test2.dat");
	int id1 = rm.openFile("test1.dat");
	int id2 = rm.openFile("test2.dat");
	for (int i=0; i<10; i++)
	{
		auto p = rm.insert(id1, RecordA());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	for (int i=0; i<10; i++)
	{
		auto p = rm.insert(id1, RecordC());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	for (int i=0; i<10; i++)
	{
		auto p = rm.insert(id1, RecordB());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	printf("%c\n", rm.get<RecordA>(id1, RecordPos(2, 1)).data[0]);
	printf("%c\n", rm.get<RecordC>(id1, RecordPos(12, 0)).data[0]);
	printf("%c\n", rm.get<RecordB>(id1, RecordPos(2, 8)).data[0]);
	for (auto const& i: rm.select<char>(id1, [](char const& p){return p == 'A';}))
		printf("[%d %d]", i.pageID, (int)i.slotID);
	printf("\n");
	for (int i=0; i<10; i++)
	{
		auto p = rm.insert(id2, RecordB());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	for (int i=0; i<10; i++)
	{
		auto p = rm.insert(id2, RecordC());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	for (int i=0; i<10; i++)
	{
		auto p = rm.insert(id2, RecordA());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	printf("%c\n", rm.get<RecordB>(id2, RecordPos(1, 9)).data[0]);
	printf("%c\n", rm.get<RecordC>(id2, RecordPos(11, 0)).data[0]);
	printf("%c\n", rm.get<RecordA>(id2, RecordPos(12, 2)).data[0]);
	for (auto const& i: rm.select<char>(id2, [](char const& p){return p == 'A';}))
		printf("[%d %d]", i.pageID, (int)i.slotID);
	printf("\n");
	return 0;
}
