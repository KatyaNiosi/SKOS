// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include <spede/machine/io.h>
#include <spede/machine/pic.h>
#include <spede/flames.h>

#define VGA (0x0f00 + '0') 

unsigned short *vid_mem_ptr = (unsigned short *) 0xB8000+5*80+229;

void ProducerProc(){
  unsigned short ch = VGA;
  if(product_num == -1){
    SemReq();
    SemPost(product_sem);
    product_num = 0;
  }
  while(1){
    int i;
    SemWait(product_sem);
    cons_printf("\n++ Producer %d producing...", GetPid());
    for(i = 0; i < 1666667; i++) IO_DELAY();
    product_num++;
    cons_printf(" product # is now %d ++", product_num);
    outportb(0x20, 0x60);
    *vid_mem_ptr = ch + product_num;//product_num;
    SemPost(product_sem);
  }
}

void ConsumerProc(){
  unsigned short ch = VGA;
  if(product_num == -1){
     SemReq();
     SemPost(product_sem);
     product_num = 0;
   }
   while(1){
     int i;
     SemWait(product_sem);
     cons_printf("\n-- Consumer %d consuming...", GetPid());
     for(i = 0; i < 1666667; i++) IO_DELAY();
     product_num--;
     cons_printf(" product # is now %d --", product_num);
     outportb(0x20, 0x60);
     *vid_mem_ptr = ch + product_num;
     SemPost(product_sem);
   }
}

void IdleProc() {
   int i;
   while(1){
     if(cons_kbhit() && cons_getchar() == 'b') breakpoint();
     cons_printf("0..");
     for(i = 0; i < 1666667; i++) IO_DELAY();
   }
}

void UserProc() {
   int sleep_period;
   sleep_period = 75 + (GetPid()-1)%4*75;

   while(1){
      cons_printf("%d..", GetPid());
      Sleep(sleep_period);
   }
}
