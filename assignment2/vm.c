/*utf-8*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

typedef struct DoubleLinkedList{
    int Table[2];
    struct DoubleLinkedList *next;
    struct DoubleLinkedList *prior;
} DLDList;

//定义全局变量
#define ADDRESS_MASK  0xFFFF  // 1111 1111 1111 1111 用来得到addresses.txt读到的页码
#define OFFSET_MASK  0xFF     // 1111 1111 用来得到addresses.txt读到的偏移

//#define NUMBER_OF_FRAMES 256  // 物理内存中帧的数量
#define FRAME_SIZE 256        // 每帧的大小

#define TLB_NUM 16
#define PAGE_TABLE_NUM 256

#define BUFFER_SIZE 32 

FILE *address_file;  //two files
FILE *backing_store;

/*链表相关操作*/
DLDList* initialization(int size);
DLDList* find(DLDList *head, int num);
DLDList* Update(DLDList *List_head, DLDList *List_current);
DLDList* Delete(DLDList *List_head, DLDList *List_current);

void InitFrame(DLDList *head);

/*get params from cmd*/
void getOptions(int* argc, char **argv, int* number_of_frames, int* policy, int* cmd, char *opt_string){
    while((*cmd = getopt(*argc, argv, opt_string)) != -1){
        switch (*cmd){
            case 'p':{
                if(strcmp(optarg,"FIFO") == 0 || strcmp(optarg,"fifo") ==0){
                    *policy = 0;  
                }
                else if(strcmp(optarg,"LRU") == 0 || strcmp(optarg,"lru") ==0){
                    *policy = 1;  //change policy
                }
                else{   
                    printf("Invaild input of -p. please use LRU(lru) or FIFO(fifo)\n");
                    exit(1);
                }
                break;
            }
            case 'n':{
                *number_of_frames = atoi(optarg);
                if(*number_of_frames > 0 && *number_of_frames <= 256)
                    ;
                else{
                    printf("Invaild, [0 < number of frame <= 256 please]\n");
                    exit(1);
                }
                break;
            }
            default:{
                exit(1);
                break;
            }
        }
    }
}

int main(int argc, char **argv){
    char buffer[BUFFER_SIZE];
    int pageTable[PAGE_TABLE_NUM];       //保存页表信息，索引为页码，值为帧码
    memset(pageTable, -1, sizeof(pageTable));

    int options;
    int policy_type = 0;  //0 means fifo, 1 means lru
    char *opt_str = "p:n:";
    int NUMBER_OF_FRAMES = 256;   //default: fifo & 256

    getOptions(&argc, argv, &NUMBER_OF_FRAMES, &policy_type, &options, opt_str); //从cmd读取参数
    
    char physicalMem[NUMBER_OF_FRAMES][FRAME_SIZE];  //physical memory
    
    address_file = fopen("addresses.txt","r");
    if(address_file == NULL){
        printf("Read addresses.txt Failed!\n");
        return -1;
    }

    backing_store = fopen("BACKING_STORE.bin","rb");
    if(backing_store == NULL){
        printf("Read BACKING_STORE.bin Failed\n");
        return -1;
    }

    DLDList *TLB = initialization(TLB_NUM);
    DLDList *physicalFrame = initialization(NUMBER_OF_FRAMES);
    
    InitFrame(physicalFrame);
    physicalFrame = physicalFrame -> prior;
    DLDList *TLB_current, *Frame_List_current;
    
    int totNumberOfAddresses = 0;
    int logical_address;
    //int physical_address;
    int pageNumber, offset, TLB_to_del;   //如果物理内存置换，需要将TLB中对应条目删掉
    int frameNumber = 0;
    int pageFaults = 0;
    int TLBHits = 0;
    
    while(fgets(buffer, BUFFER_SIZE, address_file) != NULL){  //read from addresses.txt
        logical_address = atoi(buffer);

        pageNumber = ((logical_address & ADDRESS_MASK) >> 8); //获取页码
        offset = logical_address & OFFSET_MASK; //获取偏移

        TLB_current = find(TLB, pageNumber);   //查找TLB

        if (TLB_current == NULL){    //TLB not Hits

            if (pageTable[pageNumber] == -1){   //页表中也找不到

                pageFaults += 1;
                physicalFrame = physicalFrame -> next;
                pageTable[pageNumber] = physicalFrame -> Table[1];
                frameNumber = physicalFrame -> Table[1];
                TLB_to_del = physicalFrame -> Table[0];

                if(TLB_to_del >= 0){

                    pageTable[TLB_to_del] = -1;        //从页表中删除
                    TLB_current = find(TLB, TLB_to_del); 
                    if(TLB_current != NULL){
                        TLB = Delete(TLB, TLB_current); //从TLB中删除对应帧
                    } 
                }
                
                physicalFrame -> Table[0] = pageNumber;
                fseek(backing_store, pageNumber * FRAME_SIZE, SEEK_SET); 
                
                /*从BACKING_STORE.bin中读*/
                fread(physicalMem[frameNumber], FRAME_SIZE, 1, backing_store);
                
            }
            else{
                frameNumber = pageTable[pageNumber];
                if(policy_type){
                    Frame_List_current = find(physicalFrame, pageNumber);
                    physicalFrame =  Update(physicalFrame, Frame_List_current); //LRU的page replacement
                }

            }
            TLB = TLB -> next;
            TLB -> Table[0] = pageNumber;
            TLB -> Table[1] = frameNumber;
        }
        else {
            TLBHits++;
            frameNumber = TLB_current -> Table[1];
            if(policy_type){
                TLB =  Update(TLB, TLB_current); //LRU的TLB
            }  
        } 
        // physical_address = frameNumber * 256 + offset;
        // printf("%d\n", physicalMem[frameNumber][offset]);
        totNumberOfAddresses += 1;
    }

    printf("Page Faults = %d\n", pageFaults);
    printf("TLB Hits = %d\n", TLBHits);

    double pfRate = pageFaults * 1.0 / totNumberOfAddresses;
    double TLBHitsRate = TLBHits * 1.0 / totNumberOfAddresses;
    printf("Page Fault Rate = %.3f\n", pfRate);
    printf("TLB Hit Rate = %.3f\n", TLBHitsRate);
    
    fclose(address_file);
    fclose(backing_store);
    return 0;
}

void InitFrame(DLDList *head){
    DLDList *tail = head;
    int temp = 0;
    while(head -> next != tail){
        head -> Table[1] = temp;
        temp ++;
        head = head -> next;
    }
    head -> Table[1] = temp;
}

DLDList* initialization(int size){
    int i;
    DLDList *new_node, *current_node, *head;
    new_node = (DLDList *)malloc(sizeof(DLDList));
    new_node -> Table[0] = -1;
    new_node -> Table[1] = -1;
    new_node -> prior = new_node;
    new_node -> next = new_node;
    current_node = new_node;
    head = current_node;
    for(i = 0; i < size - 1; i++){
        new_node = (DLDList *)malloc(sizeof(DLDList));
        new_node -> Table[0] = -1;
        new_node -> Table[1] = -1;
        current_node -> next = new_node;
        new_node -> prior = current_node;
        new_node -> next = head;
        head -> prior = new_node;
        current_node = new_node;
    }
    return head;
}

DLDList* find(DLDList *head, int num){
    DLDList *tail = head;
    while(head -> next != tail){
        if(head -> Table[0] == num)
            return head;
        head = head -> next;
    }
    if(head -> Table[0] == num){
        return head;
    }  
    return NULL;
}

DLDList* Update(DLDList *List_head, DLDList *List_current){

    if(List_current != List_head){
        List_current -> prior -> next = List_current -> next;  //先删除此节点
        List_current -> next -> prior = List_current -> prior;

        List_current -> next = List_head -> next; 
        List_current -> prior = List_head;
        List_head -> next -> prior = List_current;
        List_head -> next = List_current;
        List_head = List_current;   
    }
    return List_head;
}

DLDList* Delete(DLDList *List_head, DLDList *List_current){
    
    if(List_current != List_head){
        List_current -> Table[0] = -1;
        List_current -> prior -> next = List_current -> next;
        List_current -> next -> prior = List_current -> prior;
        List_current -> next = List_head -> next;
        List_current -> prior = List_head;

        List_head -> next -> prior = List_current;
        List_head -> next = List_current; 
    }
    else{
        List_head -> Table[0] = -1;
        List_head = List_head->prior;
    }
    return List_head;
}