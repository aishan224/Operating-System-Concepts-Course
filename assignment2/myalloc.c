#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <alloca.h>

#define MAX_SIZE 131072  //128kb

/*在malloc一段内存的时候，会在基本大小的基础上加上结构体mcb的大小，
而返回的分配内存地址则需要跳过结构体mcb*/
typedef struct memCtrlBlock
{
    int available;   //内存块是否可用, 1可用, 0不可用
    int size;        //内存块大小
} mcb;

void *memStart;  //分配内存时查找空闲内存的起始地址
void *endAddr;   //表示最后一个有效地内存地址
int whetherInited;  //是否已经初始化

void init();
void* myalloc(int num);
void myfree(void* start);

int main(){
    char *testp;
    testp = (char*)myalloc(64 * sizeof(char));
    printf("\nStart of heap 'testp': %p\n", testp);
    printf("End of heap 'testp': %p\n", testp + 64 * sizeof(char));

    sleep(30);
    myfree(testp);
    return 0;
}

/*设置memStart为当前堆结尾位置，而lastAddr也设置为当前堆结尾位置。*/
void init(){
    endAddr = sbrk(0);  //初始化有效内存的最尾端地址为堆尾位置sbrk(0)
    memStart = endAddr; //初始化查找空闲内存的起始地址
    whetherInited = 1;
}

/*myalloc function*/
void* myalloc(int num){
    if (num > MAX_SIZE){
        printf("Error! The memory to be allocated is too large\n");
        exit(-1);
    }
    
    if(!whetherInited){
        init();
    }
    void *current = memStart; //set 查询空闲内存的起始地址
    void *result = NULL;      //返回最终分配内存的地址

    num += sizeof(mcb);

    while(current != endAddr){
        mcb *pcurrent = current;
        if(pcurrent -> available && pcurrent -> size >= num){
            pcurrent -> available = 0;
            result = current;
            break;
        }
        current += pcurrent -> size;
    }

    if(!result){
        sbrk(num);   //调整堆尾部大小
        result = endAddr;
        endAddr += num;
        mcb *pcb = result; 
		pcb->size = num; 
		pcb->available = 0;

    }

    result += sizeof(mcb);
    return result;
}

void myfree(void* start){
    mcb *pmcb = (mcb *)(start - sizeof(mcb));
    pmcb -> available = 1;
}
