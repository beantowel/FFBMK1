#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <windows.h>
#include <math.h>
#define PI 3.1415926
typedef struct _AA{
    unsigned short a,b;
} AA;
typedef struct _BB{
    int a;
} BB;
AA aa,*aa2;
BB bb;
enum FFBEType {
    // FFB Effect Type,Order Defined in Report Descriptor
    // Refer to: Effect Type Item in Create New Effect Report
    ET_NONE = 0,      //    No Force
    ET_CONST = 1,    //    Constant Force
    ET_RAMP = 2,    //    Ramp
    ET_SQR = 3,    //    Square
    ET_SINE = 4,    //    Sine
    ET_TRNGL = 5,    //    Triangle
    ET_STUP = 6,    //    Sawtooth Up
    ET_STDN = 7,    //    Sawtooth Down
    ET_SPRNG = 8,    //    Spring
    ET_DMPR = 9,    //    Damper
    ET_INRT = 10,   //    Inertia
    ET_FRCTN = 11,   //    Friction
    ET_CSTM = 12,   //    Custom Force Data
};
uint32_t timeFunc(uint8_t idx,uint32_t kao){
    return idx+kao;
}
void testFuncPara(uint32_t tfunc(uint8_t,uint32_t)){
    printf("testFunc%ld\n",tfunc(90,10000));
}
void FuncInFunc(){
    //uint32_t funInfun(uint8_t a,uint32_t b);
    //testFuncPara(funInfun);
    //GNU C 不支持内嵌函数
}
int main(){
    aa.a=0xff; aa.b=0xef;
    aa2=new(AA);
    *aa2=aa;
    bb=*((BB*) aa2);
	printf("sturct Test:\n%d %d\n",sizeof(AA),sizeof(BB));
    printf("0x%.4x 0x%.4x\n",aa2->a,aa2->b);
    printf("0x%.8x\n",bb.a);
    printf("%d\n",ET_STUP);
    int t=3;
    switch(t){
        case 0:
            printf("0-%d\n",t);
            break;
        case 1:
        case 2:
        case 3:
            printf("1-%d\n",t);
            break;
    }
    printf("size LONG:%d\n",sizeof(LONG));
    printf("size DWORD:%d\n",sizeof(DWORD));
    printf("size Float:%d\n",sizeof(float));
    printf("0xffff=%d\n", 0xffff);
    printf("0xffffffff=%lu\n",0xffffffff);
    printf("float angle/radian%f\n",float(0x9e*2*180/0xff));
    testFuncPara(timeFunc);
}