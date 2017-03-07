#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <windows.h>
#define debugPrint //printf
typedef struct _A {
    uint8_t a, b;
} A;
typedef struct _B {
    uint8_t a;
    uint16_t b;
} B;
#pragma pack(1)
enum FFBOP {
    EFF_START   = 1ll, // EFFECT START
    EFF_SOLO    = 2ll, // EFFECT SOLO START
    EFF_STOP    = 3ll, // EFFECT STOP
};
#pragma pack()
uint8_t array[20];
A tya;
B tyb;
int main() {
    for(int i = 0; i < 20; i++) {
        array[i] = i;
        printf("0x%.2X ", i);
    }
    printf("\n");
    tya.a = 40;
    tya.b = 30;
    B *pB = (B *) &array[2];
    printf("pB:0x%.4X 0x%.4X\n", pB->a, pB->b);
    debugPrint("debugPrint\n");
    printf("ULONG %d\n", sizeof(ULONG));
    uint8_t run = 1;
    if(run) printf("running %d\n", run);
    printf("size EFF_START %d\n", sizeof(EFF_START));

    uint8_t a = -1;
    printf("0x%.2X\n", a == 0xFF);
    printf("%d:size%d\n", 5u, sizeof(5U));

    uint8_t cn = EFF_START;
    printf("%x\n", cn);

    printf("DWORD Size:%d\n", sizeof(DWORD));
    printf("LONG Size:%d\n", sizeof(LONG));
    printf("INT Size:%d\n", sizeof(INT));
    printf("WORD Size:%d\n", sizeof(WORD));

    uint8_t ma[10][10];
    for(int i = 0; i < 10; i++)
        for(int j = 0; j < 10; j++)
            ma[i][j] = i * 10 + j;
    for(int i = 0; i < 100; i++)
        printf("%d ", *(&ma[0][0] + i));
    printf("------------------\n");
    memcpy(&ma[2][0], &ma[3][0], 10);
    for(int i = 0; i < 100; i++)
        printf("%d ", *(&ma[0][0] + i));
}