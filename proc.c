// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"

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
