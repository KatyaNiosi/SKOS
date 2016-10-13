// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include "tools.h"
#include <spede/machine/io.h>
#include <spede/machine/pic.h>
#include <spede/flames.h>

#define LOOP 1666667
#define VGA (0x0f00 + '0') 

unsigned short *vid_mem_ptr = (unsigned short *) 0xB8000+5*80+229;

void PrinterProc(){
   int i, code;
   char key, *p;
   char hello[] = "Hello, World!\n\r";

   //Request for a printer_sem 
   printer_sem = DeQ(&avail_sem_q);
   
   //initalize printer port (check printer power, cable, and paper)
   outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN); //CONTROL reg, Select interupt 
   code = inportb(LPT1_BASE+LPT_STATUS);    //Ready Status
   for(i = 0; i < 50; i++) IO_DELAY();

   outportb(LPT1_BASE+LPT_CONTROL, PC_INIT | PC_SLCTIN | PC_IRQEN); // IRQ enable

   while(1){
     Sleep(1);
     if(!cons_kbhit()) continue;
       key = cons_getchar();
       switch(key){
          case 'b':
             breakpoint();
             break;
          case 'q':
             exit(0);
          default:
             cons_printf("Char: %c ", key);
       }
       if(key != '1' || key != '2') continue;
       p = hello;
       while(*p){
          outportb(LPT1_BASE+LPT_DATA, *p); // wite char to port DATA reg
          code = inportb(LPT1_BASE+LPT_CONTROL); // read port COTROL reg
          
          outportb(LPT1_BASE+LPT_CONTROL, code | PC_STROBE); // write with STROBE 
          for(i = 0; i < 50; i++) IO_DELAY(); // Need delay
          
          outportb(LPT1_BASE+LPT_CONTROL, code); // send back original code 
          if(key == '1'){
                // busy-poll port status until ready
                // if times out cons_printf a msg and break loop
                for(i = 0; i < LOOP * 3; i++){
                    code = PS_ACK & inportb(LPT1_BASE + LPT_STATUS);
                    if(code == 0) break;
                    IO_DELAY();
                }
                if(i == LOOP * 3){
                    cons_printf(">>> Time out while printing a char!\n");   
                    break;
                }
          }
          if(key == '2'){ 
                SemWait(printer_sem);
                // semaphore-wait on the printing semaphore
          }
          p++;
       }
    }
}

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
