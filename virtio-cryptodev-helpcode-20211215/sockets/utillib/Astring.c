
#include "Astring.h"
#include <stdlib.h>
#include <stdio.h>

void* safemalloc(int size)
{
	void* ptr = malloc(size);
	if(ptr == NULL)
		errorcheck(-1, -1, "failed to safemalloc");
	return ptr;
}

// string* string_constructor(const char* str)
// {
	
// 	string* s = malloc(sizeof(string));
// 	errorcheck((s==NULL), 1, "failed to allocate mem");
// 	s->charlist = emptyList;
// 	for(int i = strlen(str)-1; i>=0; i--)
// 	{
// 		char* c = safemalloc(sizeof(char));
// 		*c = str[i];
// 		s->charlist = cons(c, s->charlist);
// 	}
// 	s->length = strlen(str);
// 	return s;
// }

string* string_constructor(char* str, int size)
{
	string* s = safemalloc(sizeof(string));
	s->charlist = emptyList;
	for(int i = size-1; i>=0;i--)
	{
		char* c = safemalloc(sizeof(char));
		*c = str[i];
		s->charlist = cons(c, s->charlist);
	}
	s->length = size;
	return s;
}

void deleteNode(char* c)
{
	free(c);
}
void string_destructor(string* s)
{
	deleteList(s->charlist, (void (*)(void*))deleteNode);
	free(s);
}


char* string_tocharpointerNULLTERM(string* s)
{
	char* str = safemalloc(sizeof(char)*(s->length+1));
	int j = 0;
	forEachList(s->charlist, i)
	{
		char* c = getData(i);
		str[j++] = *c;
	}
	str[j] = 0;
	return str;
}

void string_appendStr(string* dest, string* source)
{
	forEachList(source->charlist, i)
	{
		char* c = getData(i);
		append(dest->charlist, c);	
	}
	dest->length += source->length;
}


void string_insertNthChr(string* s,char c,int n)
{
	if(n >= s->length)
	{
		printf("tried to add to last pls use append @insertNthChr\n");
		return;
	}
	if(n==0)
	{
		list* nnode = safemalloc(sizeof(list));
		char* nc = safemalloc(sizeof(char));
		*nc = c;
		nnode->data = nc;
		nnode->next = s->charlist;
		s->charlist = nnode;
		return;
	}
	int cnt = 0;
	
	list* l = s->charlist;
	forEachList(s->charlist, i)
	{
		if(!(cnt < n))
			break;
		l = i;
		cnt++;
	}
	list* nnode = safemalloc(sizeof(list));
	list * temp = l->next;
	nnode->next = temp;
	l->next = nnode;
	char* nc = safemalloc(sizeof(char));
	*nc = c;
	nnode->data = nc;
	s->length++;

}
void string_insertNthStr(string* s, string* y, int n){
	
	// list *some = reverse(y->charlist);
	char* str = string_tocharpointerNULLTERM(y);
	for(int i = strlen(str)-1; i>=0;i--)
	{
		string_insertNthChr(s, str[i], n);
	}
	s->length += y->length;
	free(str);
}


int string_strFind(string* s, char c)
{
	int cnt = 0;
	forEachList(s->charlist, i)
	{
		char* cp = getData(i);
		if(*cp == c)
			return cnt;
		cnt++;
	}
	return -1;
}

list* string_splitAt(string* s, char c)
{
	list* l = emptyList;
	list* start = s->charlist;
	forEachList(s->charlist, i)
	{

		char* cp = getData(i);
		if(*cp == c)
		{
			list* charlist = emptyList;
			int size = 0;
			for(ListIterator j = start; j!=i; j = getNext(j))
			{
				size++;
				char* cp = safemalloc(sizeof(char));
				*cp = *((char*)getData(j));
				charlist = cons(cp, charlist);
			}
			string* s = safemalloc(sizeof(string));
			s->charlist = reverse(charlist);
			s->length = size;
			l = cons(s, l); 
			start = getNext(i);
		}
	}
	if(start != emptyList)
	{
		list* charlist = emptyList;
		int size = 0;
		for(ListIterator j = start; j!=NULL; j = getNext(j))
		{
			size++;
			char* cp = safemalloc(sizeof(char));
			*cp = *((char*)getData(j));
			charlist = cons(cp, charlist);
		}
		string* s = safemalloc(sizeof(string));
		s->charlist = reverse(charlist);
		s->length = size;
		l = cons(s, l); 
	}
	return reverse(l);
}

string* string_slice(string* s, int start, int end)
{
	list* l = emptyList;
	int cnt = 0;
	forEachList(s->charlist, i)
	{
		char* cp = getData(i);
		if(cnt >= start && cnt < end)
		{
			char* nc = safemalloc(sizeof(char));
			*nc = *cp;
			l = cons(nc, l);
		}
		if(cnt >= end)
			break;
		cnt++;
	}
	string* res = safemalloc(sizeof(string));
	res->charlist =reverse(l);
	res->length = end - start;
	return res;
}


string* string_Copy(string* s)
{
	return string_slice(s, 0, s->length);
}


int string_equals(string* s, string* y)
{
	if(s->length != y->length)
		return 0;
	ListIterator j = y->charlist;
	forEachList(s->charlist, i)
	{
		char* x = getData(i);
		char* y = getData(j);
		if(*x != *y) 
			return 0;

		j = getNext(j);
	}
	return 1;
}


int string_equalsCharp(string* s, char* str)
{
	if((strlen(str))!=s->length)
		return 0;
	int j = 0;
	forEachList(s->charlist, i)
	{
		char* x = getData(i);
		if(*x!=str[j])
			return 0;
		j++;
	}
	return 1;
}



int string_countChar (string* s, char c)
{
	char* temp;
	int count = 0;
	forEachList(s->charlist, i)
	{
		temp = getData(i);
		if(*temp == c)
			count++;
	}
	return count;
}