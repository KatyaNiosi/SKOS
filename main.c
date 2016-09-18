// main.c, 159"
// this kernel is simulated, not real yet
//
// Team Name: SKOS (Members: Sayd Mateen and Katya Niosi)
// 
#include "spede.h"      // spede stuff
#include "main.h"       // main stuff
#include "isr.h"        // ISR's
#include "tools.h"      // handy functions for Kernel
#include "proc.h"       // processes such as IdleProc()
#include "typedef.h"    // data types

// kernel data stuff:
int run_pid;             // currently-running PID, if -1, none running
q_t ready_q, avail_q;    // processes ready to run and ID's un-used
pcb_t pcb[PROC_NUM];     // process table
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks

int main() {
   int new_pid;

   InitKernelData();      // to initialize kernel data
  
   new_pid = DeQ(&avail_q); // to dequeue avail_q to get an un-used pid
   NewProcISR(new_pid, IdleProc); // to create IdleProc

   //an infinite loop to alternate two functions:
   while(1) {  
      ProcLoader();      // which is to simulate loading a process to run
      KernelMain();       // to simulate kernel run periodically
   }
   return 0;             // not reached, but compiler needs it for syntax
}

void InitKernelData() {
   int i;

   MyBzero((char *)&ready_q, sizeof(ready_q));   // to clear the ready queue
   MyBzero((char *)&avail_q, sizeof(avail_q));   // to clear the available queue

   for (i = 0; i <= Q_SIZE-1; i++)
      EnQ(i, &avail_q);              // to enqueue i to avail_q

   run_pid = 0;           // IdleProc is chosen to run first
}

void ProcScheduler() {  // to choose a run PID
   if(run_pid >= 1)     //return if one of the user processes is running
     return;
   
   if(run_pid == 0)      // IdleProc is being run
      pcb[run_pid].state = READY;

   run_pid = DeQ(&ready_q);     // dequeue ready_q to get run_pid
  
   if(run_pid == -1)             // ready_q is empty
      run_pid = 0;               // fall back to run IdleProc

   pcb[run_pid].state = RUN;
   pcb[run_pid].runtime = 0;     // clear selected processes runtime to 0
}

void KernelMain() {
   int new_pid;
   char key;

   TimerISR();      //  service timer (as if it just occurred)

   if(cons_kbhit()) {
      key = cons_getchar();

      switch(key) {
         case 'n':
            new_pid = DeQ(&avail_q); // dequeue avail_q for a new_pid
            if(new_pid== -1)
              cons_printf("Kernel Panic: no more process!\n");
            else
              NewProcISR(new_pid, UserProc); // create a new process
            break;

         case 'k':
            KillProcISR();            // to kill the run_pid process
            break;

         case 'b':
            breakpoint();             //  brings up GDB prompt
            break;

         case 'q':
           exit(0);    // quit the whole thing, MyOS.dli run
     }
   }
   ProcScheduler();     // choose a new run_pid if needed
}

