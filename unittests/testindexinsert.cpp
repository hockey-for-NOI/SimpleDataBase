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
	im.insert(id1, RecordA(0, 5));
	im.insert(id1, RecordA(0, 3));
	im.insert(id1, RecordA(0, 6));
	im.insert(id1, RecordA(0, 8));
	im.insert(id1, RecordA(0, 7));
	im.insert(id1, RecordA(0, 0));
	im.insert(id1, RecordA(0, 9));
	im.insert(id1, RecordA(0, 1));
	im.insert(id1, RecordA(0, 2));
	im.insert(id1, RecordA(0, 4));
	im.insert(id1, RecordA(1, 0));
	im.insert(id1, RecordA(1, 1));
	return 0;
}
