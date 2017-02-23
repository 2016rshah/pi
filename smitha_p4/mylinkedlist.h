#include <string.h>

char* strdup(const char* s);

typedef struct LinkedList {
    char* name;
    int count;
    struct LinkedList* next;
} LinkedList;

void iterateAndPrintElementsInList(LinkedList* list) {
    LinkedList* ptr = list;
    while (ptr) {
        printf("%svar:\n   .quad 0\n", ptr->name);
        printf("%sstring:\n .string \"%s\"\n", ptr->name, ptr->name);
        ptr = ptr->next;
    }
}

int getStackCount(LinkedList* list, char* key) {
    LinkedList* ptr = list;
    while (ptr && strcmp(ptr->name, key)) {
        ptr = ptr->next;
    }
    if (ptr) {
        return ptr->count;
    }
    return -1;
}
/*
bool contains(LinkedList* list, char* key) {
    LinkedList* ptr = list;
    while (ptr && strcmp(ptr->name, key)) {
        ptr = ptr->next;
    }

    return ptr;
}

bool addToEnd(LinkedList** list, char* key, int count, bool param) {
    LinkedList** ptr = list;
    while (*ptr && strcmp((*ptr)->name, key)) {
        ptr = &((*ptr)->next);
    }
    if (*ptr) {
        return false;
    }
    *ptr = (LinkedList*) malloc(sizeof(LinkedList));
    (*ptr)->name = strdup(key);
    (*ptr)->count = 8*count; //8*count
    (*ptr)->param = param;
    (*ptr)->next = (LinkedList*)0;
    return true;
}
*/
void add(LinkedList** list, char* key, int count) {
    LinkedList** ptr = list;
    while (*ptr && strcmp((*ptr)->name, key)) {
        ptr = &((*ptr)->next);
    }
    if (!(*ptr)) {
        *ptr = (LinkedList*) malloc(sizeof(LinkedList));
        (*ptr)->name = strdup(key);
        (*ptr)->count = 8*count; //8*count
        (*ptr)->next = (LinkedList*)0;
    }
}

void freeLinkedList(LinkedList* list) {
    LinkedList* ptr;
    while (list) {
        ptr = list;
        list = list->next;
        free(ptr->name);
        free(ptr);
    }
}
