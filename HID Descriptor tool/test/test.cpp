#include <cstdio>
struct _A{
	int a;
	struct _B
	{
		int b;
	} B;
} A;
int main()
{
	A.a=1;
	A.B.b=2;
	printf("size=%d\n",sizeof(A));
    _A aa;
    aa.a=1;
    printf("%d\n",aa.a);
	return 0;
}