#include "utils/pagedef.h"
#include "utils/MyBitMap.h"

class	EnableMyBitMapInitConstDummyClass
{
public:
	EnableMyBitMapInitConstDummyClass() {MyBitMap::initConst();}
}	enableMyBitMapInitConstDummyClassInstance;

int compare = 0;
int tt = 0;
unsigned char h[61];
