#include <stdio.h>
#include <stdlib.h>
#include <time.h>

FILE* address1w;

typedef struct DoubleLinkedList{
    int value;
    int *a;
    struct DoubleLinkedList *next;
    struct DoubleLinkedList *prior;
} DLDList;

DLDList* initial(int num, FILE* address1w){
    srand(time(NULL));
    int i;
    DLDList *new_node, *current_node, *head;
    new_node = (DLDList *)malloc(sizeof(DLDList));
    new_node -> value = rand()%5000 + 1;
    new_node -> a = &(new_node->value);
    //fprintf(address1w, "%d\n", new_node->a);
    new_node -> prior = new_node;
    fprintf(address1w, "%d\n", &(new_node->next));
    new_node -> next = new_node;
    fprintf(address1w, "%d\n", &(new_node->prior));
    current_node = new_node;
    head = current_node;
    for(i = 0; i < num - 1; i++){
        new_node = (DLDList *)malloc(sizeof(DLDList));
        new_node -> value = rand()%5000 + 1;
        current_node -> next = new_node;
        new_node -> a = &(new_node->value);
        //fprintf(address1w, "%d\n", new_node->a);
        new_node-> prior = current_node;
        fprintf(address1w, "%d\n", &(new_node->next));
        new_node-> next = head;
        fprintf(address1w, "%d\n", &(new_node->prior));
        head -> prior = new_node;
        current_node = new_node;
    }
    return head;
}

int main(){
    address1w = fopen("addresses-locality.txt", "w");
    srand(time(NULL));
    initial(10000, address1w);
    return 0;
}
