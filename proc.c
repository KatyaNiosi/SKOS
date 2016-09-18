// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions

void ProcLoader() {     // load a process to run, here simulated
   if (run_pid == 0)
      IdleProc();   // as if loads it and it runs
   else             // it's one of the other processes
      UserProc();   // as if loads it and it runs
}

void IdleProc() {
   int i;

   cons_printf("0..");
   for(i = 0; i < 166666; i++) IO_DELAY();
}

void UserProc() {
   int i;
   cons_printf("%d..", run_pid);
   for(i = 0; i < 166666; i++) IO_DELAY();
}
