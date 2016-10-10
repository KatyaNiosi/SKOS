// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include <spede/machine/io.h>
#include <spede/machine/pic.h>

unsigned short *vid_mem_ptr = (unsigned short *) 0xB8000+10*80+39;

void ProducerProc(){
  if(poduct_num == -1){
    semReq();
    semPost();
    product_num = 0;
  }
  while(1){
    semWait(product_sem);
    cons_printf("\n++ Producer %d producing... ", GetPid());
    for(int i = 0; i < 1999999; i++) IO_DELAY();
    product_num++;
    cons_printf(" product # is now %d ++", product_num);
    *vid_mem_ptr = product_num;
    semPost(product_sem);
  }
}

void ConsumerProc(){
   if(product_num == -1){
     semReq();
     semPost();
     product_num = 0;
   }
   while(1){
     semWait(product_sem);
     cons_printf("\n-- Consumer %d consuming...", GetPid());
     for(int i = 0; i < 1999999; i++) IO_DELAY();
     product_num--;
     cons_printf(" product # is now %d --", product_num);
     *vid_mem_ptr = product_num;
     semPost(product_sem);
   }
}

void IdleProc() {
   int i;
   while(1){
     if(cons_kbhit() && cons_getchar() == 'b') breakpoint();
     cons_printf("0..");
     for(i = 0; i < 166666; i++) IO_DELAY();
   }
}

void UserProc() {
   int sleep_period;
   sleep_period = 500 + (GetPid()-1)%4*500;

   while(1){
      cons_printf("%d..", GetPid());
      Sleep(sleep_period);
   }
}
