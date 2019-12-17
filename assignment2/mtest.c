#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <alloca.h>

extern int etext, edata, end;
extern void aFunction(void);

int bssVar;       //未初始化的全局变量会存储在bss段中
int dataVar = 50; //初始化的全局变量则存储在数据段中


int main(int argc, char **argv){
    char *p, *heap, *nheap;

    printf("\t|etext address: %p\t\n\t|edata address: %p\t\n\t|end address: %p\n\n", &etext, &edata, &end);

    printf("Function Location:\n");
    printf("\t|Location of 'main': %p\n", main);  //看main函数的位置
    printf("\t|Location of 'aFunction': %p\n", aFunction);  //看aFunction函数位置

    printf("\nBSS Location:\n");
    printf("\t|Location of 'bssVar': %p\n", &bssVar);  //BSS段变量的地址

    printf("\nDATA Location:\n");
    printf("\t|Location of 'dataVar': %p\n", &dataVar);  //数据段变量地址

    printf("\nStack Location: \n");
    aFunction();
    printf("\n");

    p = (char*)alloca(32); //从栈中分配空间
    if(p != NULL){
        printf("\t|Location of 'Start of alloca()ed p': %p\n", p);
        printf("\t|Location of 'End of alloca()ed p'  : %p\n\n", (p + 32 * sizeof(char)));
    }

    // b = sbrk((ptrdiff_t) 32);
    // nb = sbrk((ptrdiff_t) 0);

    heap = (char*)malloc(32 * sizeof(char)); // 从堆中分配
    nheap = (char*)malloc(16 * sizeof(char));

    printf("\nHeap Location:\n");
    printf("\t|Location of 'Start of heap': %p\n", heap);
    printf("\t|Location of 'End of heap': %p\n", (nheap + 16 * sizeof(char)));

    printf("\np, heap, nheap in Stack:\n");
    printf("\t|Location of 'p': %p\n", p);
    printf("\t|Location of 'heap': %p\n", heap);
    printf("\t|Location of 'nheap': %p\n", nheap);

    free(heap);
    free(nheap);

    sleep(60000);
    
    return 0; 
}

void aFunction(void){
    static int level = 0; //初始化为0的静态数据存储在BSS段中

    int stackVar; //局部变量，存储在栈中

    if(++level == 5){
        return;
    }

    printf("\t|Location of 'stackVar in stack section': %p\n", &stackVar); //
    printf("\t|Location of 'level in bss section': %p\n", &level);

    aFunction();
}

