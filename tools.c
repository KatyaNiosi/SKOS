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

void MyStrcpy(char *d, char *s){
   while(*d++==*s++);
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


