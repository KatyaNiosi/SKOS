// tools.c, 159

#include "spede.h"
#include "typedef.h"
#include "extern.h"

void MyBzero(char *p, int size) { // initialize the string *p to 0s
   while (size != 0){
     *p = 0;
     p++;
     size--;
   }
}

void MyStrcpy(char *dest, char *src){
  while(*src != '\0') *dest++ = *src++;
  *dest = '\0';
}

int MyStrcmp(char *p, char *q, int size){
   //while((*d++=*s++)!=0);
   //*d = '\0';
   while(size && *p != '\0' && *p == *q){
      p++; q++; size--;
   }
   if(size == 0 || (*p == '\0' && *q == '\0'))
     return 1;
   else
     return 0;
}

int MyStrlen(char *p){
   int size = 0;
   while(*p){ size++; p++; }
   return size;
}

void MyMemcpy(char *dest, char *src, int size){
/*  while(size){   // && *src != '\0'){
     *dest = *src;
     dest++; src++; size --;
  }*/
  int i;

  for(i=0; i<size; i++)
     dest[i] = src[i];
}


void Attr2Str(attr_t *attr_p, char *str){
   char *name = (char *) (attr_p + 1);

   sprintf(str, " ---- size=%4d name=%s\n", attr_p->size, name);
   if(A_ISDIR(attr_p->mode) ) str[1] = 'd';
   if(QBIT_ON(attr_p->mode, A_ROTH) ) str[2] = 'r';
   if(QBIT_ON(attr_p->mode, A_WOTH) ) str[3] = 'w';
   if(QBIT_ON(attr_p->mode, A_XOTH) ) str[4] = 'x';

}

void EnQ(int data, q_t *p) {
  // make sure the pointer is not NULL
  if(p == NULL){
      cons_printf("NULL pointer was received");
      exit(1);
  }
  // show error msg and return if queue's already full
  if (p->size == Q_SIZE){
      cons_printf("The available queue is aleady full");
      breakpoint();
      return;
  }

  p->q[p->size] = data;
  p->size++;
}

int DeQ(q_t *p) { // return -1 if q is empty
   int dequeue_proc, i;

   // ensure the pointer is not NULL
   if(p == NULL){
     cons_printf("NULL pointer received");
     exit(1);
   }
   // return -1 if q is empty
   if(p->size == 0)
     return -1;

   dequeue_proc = p->q[0]; //save the process to be dequeued

   p->size--;
   //decrement available queue size
 
   for (i = 0; i < p->size; i++)
     p->q[i] = p->q[i+1];   //shift the queue by 1 to the head


   return dequeue_proc;

}


