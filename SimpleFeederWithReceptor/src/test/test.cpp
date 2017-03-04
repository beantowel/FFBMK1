#include <cstdio>
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
int main(){
    aa.a=0xff; aa.b=0xef;
    aa2=new(AA);
    *aa2=aa;
    bb=*((BB*) aa2);
	printf("%d %d\n",sizeof(AA),sizeof(BB));
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
}