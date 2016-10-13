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
#include "entry.h"

// kernel data stuff:
int run_pid;             // currently-running PID, if -1, none running
int system_time;
q_t ready_q, avail_q;    // processes ready to run and ID's un-used
pcb_t pcb[PROC_NUM];     // process table
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks
struct i386_gate *IDT_ptr;
sem_t sem[Q_SIZE];
q_t avail_sem_q;
int product_sem, product_num, printer_sem;

int main() {
   int new_pid;

   InitKernelData();      // to initialize kernel data
   InitKernelControl(); 

   new_pid = DeQ(&avail_q); // to dequeue avail_q to get an un-used pid
   NewProcISR(new_pid, IdleProc); // to create IdleProc
   
   ProcLoader(pcb[run_pid].TF_p);//load/ IdleProc
  
   return 0;             // not reached, but compiler needs it for syntax
}

void SetEntry(int entry_num, func_ptr_t func_ptr){
    struct i386_gate *gateptr = &IDT_ptr[entry_num];
    fill_gate(gateptr, (int)func_ptr, get_cs(), ACC_INTR_GATE, 0);
}

void InitKernelControl(){
    IDT_ptr = get_idt_base();
    SetEntry(32, TimerEntry);
    outportb(0x21, ~1);

    SetEntry(48, GetPidEntry);
    SetEntry(49, SleepEntry);
    SetEntry(50, SemReqEntry);
    SetEntry(51, SemWaitEntry);
    SetEntry(52, SemPostEntry);
}

void InitKernelData() {
   int i;

   system_time = 0;     

   MyBzero((char *)&ready_q, sizeof(ready_q));   // to clear the ready queue
   MyBzero((char *)&avail_q, sizeof(avail_q));   // to clear the available queue
   MyBzero((char *)&avail_sem_q, sizeof(avail_sem_q));

   //clear avail_sem_q queue and fill it with available semaphore
   //ID's (0 to Q_SIZE-1).

   for (i = 0; i <= Q_SIZE-1; i++)
   { 
        EnQ(i, &avail_q);              // to enqueue i to avail_q
        EnQ(i, &avail_sem_q);
   }
   run_pid = 0;           // IdleProc is chosen to run first
   product_num = -1;
   product_sem = 0;
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

void KernelMain(TF_t *TF_p) {
   char key;
   int new_pid;
   // First save the TF_p into the PCB of the current run process 
   pcb[run_pid].TF_p = TF_p;
    
   switch(TF_p->intr_id){
      case TIMER_INTR:
          TimerISR();      //  service timer (as if it just occurred)
          // dismiss timer event ( send pic code as in timer intrupt lab ) 
          outportb(0x20, 0x60);
          break;

      case GETPID_INTR:
          GetPidISR();
          break;
      
      case PRINTER_INTR:
          PrinterISR();
          outportb(0x20, 0x67); // Dismiss IRQ-7 is 39
          break;

      case SLEEP_INTR: 
          SleepISR();
          break;
      
      case SEMREQ_INTR:
          SemReqISR();
          break;

      case SEMWAIT_INTR:
          SemWaitISR(TF_p->eax);
          break;

      case SEMPOST_INTR:
          SemPostISR(TF_p->eax);
          break;

      default:
          cons_printf("Kernel Panic: unknown intr ID (%d)!\n", TF_p->intr_id);
          breakpoint();
   }

   if(cons_kbhit()) {
      key = cons_getchar();

      switch(key) {
         /*case 'n':
            new_pid = DeQ(&avail_q); // dequeue avail_q for a new_pid
            if(new_pid== -1)
              cons_printf("Kernel Panic: no more process!\n");
            else
              NewProcISR(new_pid, UserProc); // create a new process
            break;
         */
         case 'p':
            // Producer
            new_pid = DeQ(&avail_q); // dequeue avail_q for a new_pid
            if(new_pid== -1)
              cons_printf("Kernel Panic: no more process!\n");
            else
              NewProcISR(new_pid, ProducerProc); // create a new process
            break;
         case 'c':
            // Consumer
            new_pid = DeQ(&avail_q); // dequeue avail_q for a new_pid
            if(new_pid== -1)
              cons_printf("Kernel Panic: no more process!\n");
            else
              NewProcISR(new_pid, ConsumerProc); // create a new process
            break;
         /*case 'k':
            KillProcISR();            // to kill the run_pid process
            break;
         */
         case 'b':
            breakpoint();             //  brings up GDB prompt
            break;

         case 'q':
           exit(0);    // quit the whole thing, MyOS.dli run
     }
   }
   ProcScheduler();     // choose a new run_pid if needed
   ProcLoader(pcb[run_pid].TF_p);
}

