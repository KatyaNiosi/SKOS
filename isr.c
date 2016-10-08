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

   // Step 2 Below
   // Set TF_p of PCB to highest mem location of the proc stack
   // then fill out its trap frame
   pcb[new_pid].TF_p = (TF_t *)&proc_stack[new_pid][4032]; // correct mem address in the stack

   pcb[new_pid].TF_p->eflags = EF_DEFAULT_VALUE | EF_INTR;   
   pcb[new_pid].TF_p->cs = get_cs();
   pcb[new_pid].TF_p->ds = get_ds();
   pcb[new_pid].TF_p->es = get_es();
   pcb[new_pid].TF_p->fs = get_fs();
   pcb[new_pid].TF_p->gs = get_gs();

   pcb[new_pid].TF_p->eip = (unsigned int)p; //set to where function pointer points to

}

void SemReqISR(){

}

void SemWaitISR(int sem_id){

}

void SemPostISR(int sem_id){

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
   int i;
   
   system_time++;

   for(i=0; i<=PROC_NUM; i++)
   {
     if(pcb[i].state == SLEEP &&
       pcb[i].wake_time == system_time)
     {
         EnQ(i, &ready_q);    //append its PID to ready queue
         pcb[i].state = READY;
      }
   }

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

void GetPidISR() {
  pcb[run_pid].TF_p->eax = (unsigned int)run_pid;
}

void SleepISR() {
  pcb[run_pid].wake_time = system_time + pcb[run_pid].TF_p->eax;
  pcb[run_pid].state = SLEEP;
  run_pid = -1;
}
