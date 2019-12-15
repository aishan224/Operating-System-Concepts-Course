/** utf-8 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//定义全局变量
#define ADDRESS_MASK  0xFFFF  // 1111 1111 1111 1111 用来得到addresses.txt读到的页码
#define OFFSET_MASK  0xFF     // 1111 1111 用来得到addresses.txt读到的偏移

#define NUMBER_OF_FRAMES 256  // 物理内存中帧的数量
#define FRAME_SIZE 256        // 每帧的大小

#define TLB_NUM 16       // TLB条目数量
#define PAGE_TABLE_NUM 256  // size of the page table

#define BUFFER_SIZE 10   // 一次从addresses.txt中读取到的字符数量
#define READ_NUM 256     // 从BACKING_STORE.bin中一次读取的

int pageTableNums[PAGE_TABLE_NUM];    // 保存页表中的页码
int pageTableFrames[PAGE_TABLE_NUM];   // 保存页表中的帧码

int TLBPageNum[TLB_NUM];   // TLB页码部分
int TLBFrameNum[TLB_NUM];  // TLB帧码部分

int physicalMem[NUMBER_OF_FRAMES][FRAME_SIZE]; // physical memory

int pageFaults = 0;   // number of page faults
int TLBHits = 0;      // number of TLB hits
int FrameIndex = 0;   // 物理内存可用帧下标
int PageTableNumberInx = 0;  // 页表可用帧下标
int TLBEntriesNums = 0;   // track the number of entries in the TLB

FILE *address_file;  //addresses.txt
FILE *backing_store; //BACKING_STORE.bin

char address[BUFFER_SIZE];  //存从addresses读到的地址数据
int  logical_address;

signed char buffer[READ_NUM];  //read from BACKING_STORE

signed char value; // physical memory中具体的值

void getPage(int address);
void readFromStore(int pageNumber);
void insertIntoTLB(int pageNumber, int frameNumber);

int main(int argc, char *argv[])
{
    address_file = fopen(argv[1], "r"); //addresses.txt
    if (address_file == NULL) {
        printf("Read %s Failed\n", argv[1]);
        return -1;
    }

    backing_store = fopen("BACKING_STORE.bin", "rb");
    if (backing_store == NULL) {
        printf("Read %s Failed\n", "BACLING_STORE.bin");
        return -1;
    }
    
    int numberOfReadAddresses = 0;
    while ( fgets(address, BUFFER_SIZE, address_file) != NULL) {
        logical_address = atoi(address);
        // 从物理内存中读取
        getPage(logical_address);
        numberOfReadAddresses++;  // 从addresses.txt已经读到的地址数加1       
    }

    printf("Page Faults = %d\n", pageFaults);
    printf("TLB Hits = %d\n", TLBHits);

    // double pfRate = pageFaults * 1.0 / numberOfReadAddresses;
    // double TLBRate = TLBHits * 1.0 / numberOfReadAddresses;
    // printf("Page Fault Rate = %.3f\n",pfRate);
    // printf("TLB Hit Rate = %.3f\n", TLBRate);

    fclose(address_file);
    fclose(backing_store);
    return 0;
}

void getPage(int logical_address){
    
    int pageNumber = ((logical_address & ADDRESS_MASK)>>8);  //前8位
    int offset = (logical_address & OFFSET_MASK);            //后8位，偏移
    
    int frameNumber = -1; //初始化为-1并在后面的操作中更新
    
    int i;
    for(i = 0; i < TLB_NUM; i++){
        if(TLBPageNum[i] == pageNumber){   // 在TLB中找到
            frameNumber = TLBFrameNum[i];  // 将TLB中的帧码赋给frameNumber
                TLBHits++; 
        }
    }
    
    if(frameNumber == -1){   //上面遍历完还没找到
        int i;  
        for(i = 0; i < PageTableNumberInx; i++){
            if(pageTableNums[i] == pageNumber){         // 在页表中找到
                frameNumber = pageTableFrames[i];     
            }
        }
        if(frameNumber == -1){              // 页表中还没找到，page fault，就要从BACKING_STORE.bin中读取
            readFromStore(pageNumber);   
            pageFaults++;      
            frameNumber = FrameIndex - 1;
        }
    }
    
    insertIntoTLB(pageNumber, frameNumber);     // 插入到TLB中
    value = physicalMem[frameNumber][offset];   // 读值

    // printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, (frameNumber << 8) | offset, value);
    printf("%d\n", value);
}

void readFromStore(int pageNumber){
    if (fseek(backing_store, pageNumber * READ_NUM, SEEK_SET) != 0) {
        printf("Seeking in backing store Failed\n");
    }
    
    // 读取到buffer中
    if (fread(buffer, sizeof(signed char), READ_NUM, backing_store) == 0) {
        printf("Reading from backing store Failed\n");        
    }
    
    // 存到第一个可用的物理内存帧中
    int i;
    for(i = 0; i < READ_NUM; i++){
        physicalMem[FrameIndex][i] = buffer[i];
    }
    
    // 更新页表可用帧
    pageTableNums[PageTableNumberInx] = pageNumber;
    pageTableFrames[PageTableNumberInx] = FrameIndex;
    
    FrameIndex++;
    PageTableNumberInx++;
}

void insertIntoTLB(int pageNumber, int frameNumber){
    
    int i; 
    for(i = 0; i < TLBEntriesNums; i++){
        if(TLBPageNum[i] == pageNumber){
            break;
        }
    }
    
    if(i == TLBEntriesNums){  
        if(TLBEntriesNums < TLB_NUM){  // 还有空间
            TLBPageNum[TLBEntriesNums] = pageNumber;    // 直接插到下面
            TLBFrameNum[TLBEntriesNums] = frameNumber;
        }
        else{                                        // 不然第一个不要了，其他上移，插到最下面(FIFO)
            for(i = 0; i < TLB_NUM - 1; i++){
                TLBPageNum[i] = TLBPageNum[i + 1];
                TLBFrameNum[i] = TLBFrameNum[i + 1];
            }
            TLBPageNum[TLBEntriesNums-1] = pageNumber;  
            TLBFrameNum[TLBEntriesNums-1] = frameNumber;
        }        
    }

    if(TLBEntriesNums < TLB_NUM){     
        TLBEntriesNums++;
    }    
}

