// isr.c, 159

#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "tools.h"
#include "extern.h"
#include "proc.h"

void NewProcISR(int new_pid, func_ptr_t p) {
   // clear the PCB and proc_stack for this new process
   MyBzero((char *)&pcb[new_pid], sizeof(pcb_t));
   MyBzero(&proc_stack[new_pid][0], PROC_STACK_SIZE);

   // set the state in its PCB to READY
   pcb[new_pid].state = READY;

   // enqueue new_pid to ready_q if it's > 0
   if (new_pid >0)
     EnQ(new_pid, &ready_q);
}

void KillProcISR() {
   if(run_pid == 0)   // IdleProc is exempted
      return;

   // change state of running process to AVAIL
   pcb[run_pid].state = AVAIL;

   // queue the running PID to available queue
   EnQ(run_pid, &avail_q);

   // set running PID to -1 (now none running)
   run_pid = -1;
}        

void TimerISR() {
   if(run_pid < 1)
     return;

   // upcount its runtime and lifespan (in its PCB)
   pcb[run_pid].runtime++;
   pcb[run_pid].lifespan++;

   if(pcb[run_pid].runtime == TIME_SLICE){
      pcb[run_pid].state = READY;
      EnQ(run_pid, &ready_q);    // queue its PID (run_pid) to ready_q
      run_pid = -1;             // no processes are running now
    }
}
