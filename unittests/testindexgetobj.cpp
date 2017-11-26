#include "indexman.h"
#include <cstdio>
#include <cstring>

using namespace SimpleDataBase;

struct	RecordA
{
	static	int	cnt;
	uchar data[1000];
	RecordA(unsigned int key, unsigned short subkey) {
		memset(data, 'A', sizeof(data));
		((ushort*)data)[0] = 1000;
		memcpy(data + 2, &key, 4);
		memcpy(data + 6, &subkey, 2);
		cnt++;
		memcpy(data + 8, &cnt, 4);
	}
};

int	RecordA::cnt = 1000;

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
	for (auto i: im.get<int>(id1, RecordPos(10, 1), [](void const* ptr){
		int x;
		memcpy(&x, ((uchar const*)ptr) + 8, 4);
		return x;
	})) printf("%d ", i); printf("\n");
	printf("%d\n", im.remove(id1, RecordPos(10, 1), [](void const* ptr){
		int x;
		memcpy(&x, ((uchar const*)ptr) + 8, 4);
		return x > 1010 && x < 1022;
	}));
	for (auto i: im.get<int>(id1, RecordPos(10, 1), [](void const* ptr){
		int x;
		memcpy(&x, ((uchar const*)ptr) + 8, 4);
		return x;
	})) printf("%d ", i); printf("\n");
	printf("%d\n", im.remove(id1, RecordPos(10, 2), [](void const* ptr){
		int x;
		memcpy(&x, ((uchar const*)ptr) + 8, 4);
		return x > 1010 && x < 1022;
	}));
	return 0;
}
