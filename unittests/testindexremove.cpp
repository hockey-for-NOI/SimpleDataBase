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
	im.ins(id1, RecordA(10, 0));
	for (int i=0; i<17; i++) im.ins(id1, RecordA(10, 1));
	for (int i=0; i<35; i++) im.ins(id1, RecordA(10, 2));
	printf("%d\n", im.remove(id1, RecordPos(10, 1)));
	printf("%d\n", im.remove(id1, RecordPos(10, 2)));
	return 0;
}
