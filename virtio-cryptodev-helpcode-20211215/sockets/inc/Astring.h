#include "string.h"
#include "linkedlist.h"


typedef struct{
	list* charlist;
	int length;
} string;

/*
 * Creates a string by using the size characters from str
 * if constructed from a const or null terminated list strlen()'s size must be used
 * @param str the characters to be copied
 * @param the length of characters to copy
 * @return the resulting string*
 */
string* string_constructor(char* str, int size);

/*
 * Deallocates the memory and frees s
 * @param s the string to be destructed
 */
void string_destructor(string* s);

/*
 * Creates a new null terminated char* that has the same chars as string
 * The memory in the results needs to be manually freed.
 * @param s the input string
 * @return a pointer to the newly allocated memory with characters from s [NEEDS TO BE FREED]
 */
char* string_tocharpointerNULLTERM(string* s);

/*
 * Appends source to dest string.
 * @param dest the modified string
 * @param source the string to be appended
 */
void string_appendStr(string* dest, string* source);

/*
 * Inserts char c to the n-th place of string
 * @param s the string to be modified
 * @param c the character to be inserted
 * @param n the place c is going to be placed [0 is at the front]
 */
void string_insertNthChr(string* s,char c,int n);

/*
 * Inserts string y at n-th place of string s
 * @param s the string to be modified
 * @param y the string to be inserted
 * @param n the place to insert
 */
void string_insertNthStr(string* s, string* y, int n);
/*
 * Checks if a char is in a string
 * @param s the string to be checked
 * @param c the char to find
 * @return if the character is found then the place of it in s is returned [0 for the first] else -1
 */
int string_strFind(string* s, char c);

/*
 * Splits a string into new strings at the character specified. The character c is not included
 * in the resulting strings. 
 * @param s the string to be split [Actually not modified]
 * @param c the character to find
 * @return a list* of new strings resulting from the split.
 */
list* string_splitAt(string* s, char c);

/*
 * Returns a string as slice of the original string from start to end.
 * The characters included are [start, end) and the first char is 0.
 * @param s the str to be sliced [Actually not modified]
 * @param start the first character number of the split
 * @param end the end limit
 * @return A new string with [start, end) characters.
 */
string* string_slice(string* s, int start, int end);

/*
 * Copied a string and gives the result.
 * Basically, a copy constructor.
 * @param s the string to be copied
 * @return An exact copy of s
 */
string* string_Copy(string* s);

/*
 * Compares two strings
 * @param s first string
 * @param y second string
 * @return 1 if they have the same characters at every place and same length and 0 otherwise
 */
int string_equals(string* s, string* y);

/*
 * Compares a string with the characters in a NULL TERMINATED char*.
 * @param s the string 
 * @param str the char* to be compared
 * @return 1 if s and str have the same characters at every place and same length and 0 otherwise
 */
int string_equalsCharp(string* s, char* str);

/*
 * takes in a string as an arguments and counts the number c is in string.
 * @param s the input string
 * @param c the char to count;
 * @return number of occurencies of c in s;
 */
int string_countChar (string* s, char c);

/*
 * Removes the nth character of a string.
 * @param s a pointer to the string * to be cleansed of n.
 * @param n the place of the character to be removed.
 * @return the result is in s it is passed by reference.
 */
void string_removeNthChar(string** s, int n);

/*
 * Removes All Occurencies of a char in a string.
 * @param s the string to be filtered.
 * @param c the char to be removed.
 */
void string_filter(string** s, char c);


/*
 * Finds the index of the first occurence of c in s.
 * @param s the string to be searched.
 * @param c the char to find.
 * @return The index of the first occurence of c in s of -1 if not found
 */
int string_findIndexChar(string* s, char c);