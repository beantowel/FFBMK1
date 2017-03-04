#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <basetyps.h>
// #include <cfgmgr32.h>
// #include <Setupapi.h>
// #include <strsafe.h>
// #include <Newdev.h>
// #include <INITGUID.H>

struct Byte3{
	BYTE a;
	BYTE b;
	BYTE c;
};
struct Byte4{
	BYTE a;
	BYTE b;
	BYTE c;
	BYTE d;
};

int a[10];
int main(){
	int i=0;
	for(i=0;i<10;i++){
		a[i]=i;
	}
	struct Byte4 *byte4;
	struct Byte3 *byte3;
	byte4=(Byte4 *) (a+3);
	byte3=(Byte3 *) (a+1);
	printf("sizeof a %d\n", sizeof(a));
	printf("byte3: a %d b %d c %d\n", byte3->a,byte3->b,byte3->c);
	printf("byte4: a %d b %d c %d d %d\n", byte4->a,byte4->b,byte4->c,byte4->d);

	UINT a=0x1111;
	BYTE b=010;
	printf("a %x b %2x\n",a,b);
	printf("what the fuck\n");

	return 0;
}