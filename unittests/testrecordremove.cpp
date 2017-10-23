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
	rm.remove(id1, RecordPos(1, 2));
	rm.remove(id1, RecordPos(1, 3));
	rm.remove(id1, RecordPos(1, 5));
	rm.remove(id1, RecordPos(1, 7));
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
	for (int i=0; i<10; i++)
	{
		auto p = rm.insert(id2, RecordB());
		printf("%d %d\n", p.pageID, (int)(p.slotID));
	}
	rm.remove(id2, RecordPos(1, 2));
	rm.remove(id2, RecordPos(1, 3));
	rm.remove(id2, RecordPos(1, 5));
	rm.remove(id2, RecordPos(1, 7));
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
	return 0;
}
