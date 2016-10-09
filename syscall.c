// syscall.c
// software interrupt/syscalls, i.e., kernel service API
#include "syscall.h"

int GetPid() {                   // no input, has return
   int pid;

   asm("int $48; movl %%eax, %0" // CPU inst
       : "=g" (pid)              // output from asm("...")
       :                         // no input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after

   return pid;
}

void Sleep(int centi_seconds) {   // has input, no return

   asm("movl %0, %%eax; int $49" // CPU inst
       :                         // no output from asm("...")
       : "g" (centi_seconds)      // input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after
}

int SemReq(){
   int num;

   asm("int $50; movl %%eax, %0"
      : "=g" (num)
      :
      : "%eax");

    return num;
}

void SemWait(int num){
   asm("movl %0, %%eax; int $51"
      :
      : "g" (num) 
      : "%eax");

}

void SemPost(int num){
   asm("movl %0, %%eax; int %52"
      :
      : "g" (num)
      : "%eax");

}
