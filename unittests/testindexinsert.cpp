#include "indexman.h"
#include <cstdio>
#include <cstring>

using namespace SimpleDataBase;

struct	RecordA
{
	uchar data[1000];
	RecordA(unsigned int key, unsigned short subkey) {
		memset(data, 'A', sizeof(data));
		((ushort*)data)[0] = 1000;
		memcpy(data + 2, &key, 4);
		memcpy(data + 6, &subkey, 2);
	}
};

int	main()
{
	IndexManager im;
	im.createIndex("test1.dat");
	im.createIndex("test2.dat");
	int id1 = im.openIndex("test1.dat");
	int id2 = im.openIndex("test2.dat");
	im.ins(id1, RecordA(10, 5));
	im.ins(id1, RecordA(10, 3));
	im.ins(id1, RecordA(10, 6));
	im.ins(id1, RecordA(10, 8));
	im.ins(id1, RecordA(10, 7));
	im.ins(id1, RecordA(10, 0));
	im.ins(id1, RecordA(10, 9));
	im.ins(id1, RecordA(10, 1));
	im.ins(id1, RecordA(10, 2));
	im.ins(id1, RecordA(10, 4));
	im.ins(id1, RecordA(1, 0));
	im.ins(id1, RecordA(1, 1));
	im.ins(id1, RecordA(1, 2));
	im.ins(id1, RecordA(1, 3));
	im.ins(id1, RecordA(1, 4));
	return 0;
}
