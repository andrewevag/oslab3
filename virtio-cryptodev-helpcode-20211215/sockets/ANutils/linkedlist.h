#pragma once


#define emptyList NULL
struct node;
typedef struct node{
    void* data;
    struct node* next;
} list;

typedef list* ListIterator;

#define iterateListWith(l, i) for(ListIterator i = l; i!=NULL; i=getNext(i))
#define forEachList(l,i) for(ListIterator i = l; i!=NULL; i=getNext(i))

void* head(list* l);
list* tail(list* l);
list* cons(void* data, list* l);
int listlength(list* l);
void printList(list* l, void (*printItem)(void*));
void deleteList(list* l, void (*deleteItem)(void*));
list* reverse(list* l);
list* copy(list* l);
list* append(list* l, void* data);

ListIterator getIterator(list* l);
void* getData(ListIterator i);
ListIterator getNext(ListIterator curr);
int isLast(ListIterator curr);

int check(list* l, int *(predicate)(void*));
void map(list* l, void *(fun)(void*));
void* find(list*l , int *(predicate)(void*));



int errorcheck(int val, int targetval, char* msg);