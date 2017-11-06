#include "recordman.h"
#include <cstdio>
#include <cstring>

using namespace SimpleDataBase;

struct	RecordA
{
	uchar data[1000];
	RecordA() {memset(data, 'A', sizeof(data)); ((ushort*)data)[0] = 1000;}
};

struct	RecordB
{
	uchar data[500];
	RecordB() {memset(data, 'B', sizeof(data)); ((ushort*)data)[0] = 500;}
};

struct	RecordC
{
	uchar data[8186];
	RecordC() {memset(data, 'C', sizeof(data)); ((ushort*)data)[0] = 8186;}
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
		auto p = rm.ins(id1, RecordA());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	for (int i=0; i<10; i++)
	{
		auto p = rm.ins(id1, RecordC());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	for (int i=0; i<10; i++)
	{
		auto p = rm.ins(id1, RecordB());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	printf("%c\n", rm.get<RecordA>(id1, RecordPos(2, 1)).data[2]);
	printf("%c\n", rm.get<RecordC>(id1, RecordPos(12, 0)).data[2]);
	printf("%c\n", rm.get<RecordB>(id1, RecordPos(2, 8)).data[2]);
	for (auto const& i: rm.select<uchar>(id1, [](uchar const& p){return p == (1000 & 255);}))
		printf("[%d %d]", i.pageID, (int)i.slotID);
	printf("\n");
	for (int i=0; i<10; i++)
	{
		auto p = rm.ins(id2, RecordB());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	for (int i=0; i<10; i++)
	{
		auto p = rm.ins(id2, RecordC());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	for (int i=0; i<10; i++)
	{
		auto p = rm.ins(id2, RecordA());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	printf("%c\n", rm.get<RecordB>(id2, RecordPos(1, 9)).data[2]);
	printf("%c\n", rm.get<RecordC>(id2, RecordPos(11, 0)).data[2]);
	printf("%c\n", rm.get<RecordA>(id2, RecordPos(12, 2)).data[2]);
	for (auto const& i: rm.select<uchar>(id2, [](uchar const& p){return p == (1000 & 255);}))
		printf("[%d %d]", i.pageID, (int)i.slotID);
	printf("\n");
	return 0;
}
