#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void* head(list* l){
    if(l == NULL) return NULL;
    return l->data;
}

list* tail(list* l){
    return l->next;
}

list* cons(void* data, list* l){
    list* newList = malloc(sizeof(list));
    if(newList == NULL)
    {
        errorcheck(-1, -1, "failed to allocate new memory for cons");
    }
    newList->data = data;
    newList->next = l;
    return newList;
}

int listlength(list* l){
    list* i = l;
    int counter = 0;
    while(i!=NULL){
        counter++;
        i = i->next;
    }
    return counter;
}

void printList(list* l, void (*printItem)(void*)){
    printf("[");
    list* i = l;
    while(i != NULL){
        (*printItem)(i->data);
        i = i->next;
        if (i!=NULL) printf(",");
    }
    printf("]\n");
}

void deleteList(list* l, void (*deleteItem)(void*)){
    list* i = l;
    list*  temp;
    while(i!=NULL){
        deleteItem(i->data);
        temp = i;
        i = i->next;
        free(temp);
    }
}
list* append(list* l, void* data){
    if (l == NULL){
        return cons(data,l);
    }
    list* i = l;
    while(i ->next != NULL){
        i = i->next;
    }
    i->next = cons(data,NULL);
    return l;
}
list* reverse(list* l){
    
    list* i = l;
    list* temp = NULL;
    while(i!=NULL){
        temp = cons(head(i),temp);
        i= i->next;
    }
    
    return temp;
}

list* copy(list* l){
    if (l == NULL) return NULL;
    else return cons(head(l),copy(tail(l)));
}


ListIterator getIterator(list* l){
    return (ListIterator)l;
}
ListIterator getNext(ListIterator curr){
    if(curr!=NULL) return curr->next;
    else return NULL;
}
int isLast(ListIterator curr){
    if(curr==NULL) return true;
    else return false;
}
void* getData(ListIterator i){
    return (i!=NULL)?(i->data):NULL;
}

//find the first item that yeilds true to the predicate
void* find(list* l, int *(predicate)(void*))
{
    iterateListWith(l, i)
    {
        if(predicate(getData(i))){
            return getData(i);
        }
    }
    return NULL;
}


int check(list* l, int *(predicate)(void*))
{
    iterateListWith(l, i)
    {
        if(predicate(getData(i)))
            return 1;
    }
    return 0;
}
void map(list* l, void *(fun)(void*))
{
    iterateListWith(l, i)
    {
        fun(getData(i));
    }
}




int errorcheck(int val, int targetval, char* msg)
{
	if(val == targetval)
	{
		perror(msg);
		exit(EXIT_FAILURE);
	}
	return val;
}


void* list_getNth(list *l, int n)
{
    if(listlength(l) <= n)
        return NULL;
    int count = 0;
    forEachList(l, i)
    {
        if(count == n)
        {
            return getData(i);
        }
        count++;
    }
    return NULL;
}