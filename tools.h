// tools.h, 159

#ifndef _TOOLS_H
#define _TOOLS_H

#include "typedef.h" // q_t needs be defined in code below

void MyBzero(char *, int);
void MyStrcpy(char *, char *);
int MyStrcmp(char *, char *, int);
int MyStrlen(char *);
void MyMemcpy(char *, char *, int);
int DeQ(q_t *);
void EnQ(int, q_t *);
void Attr2Str(attr_t *, char *); 
#endif

