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
   asm("movl %0, %%eax; int $52"
      :
      : "g" (num)
      : "%eax");

}

void TripTermIRQ(){
  asm("int $35");
}


int Fopen(char *name){
  int fd;
  asm("movl %1, %%eax; int $54; movl %%ebx, %0"
      : "=g" (fd)
      : "g" ((int) name)
      : "%eax" , "%ebx");
  return fd;
}

void Fstat(char *name, char *data){
  asm("movl %0, %%eax; movl %1, %%ebx; int $53"
      :
      :"g" ((int)name), "g" ((int)data)
      :"%eax" , "%ebx");
}

void Fread(int fd, char *data){
  asm("movl %0, %%eax; movl %1, %%ebx; int $55"
      : 
      :"g" (fd) , "g" ((int)data)
      :"%eax" , "%ebx");
}

void Fclose(int fd){
  asm("movl %0, %%eax; int $56"
      :
      :"g" (fd)
      : "%eax");
}

//write to STdouT (terminal)
void SysWrite(char *p){
  asm("movl %0, %%eax; int $57"
      : 
      : "g" ((int) name)
      : "%eax");
}

// create a.out process
void Fork(char *code_addr, int code_size){
  asm("movl %0, %%eax; movl %1, %%ebx; int $58"
     :
     :"g"((int)code_addr), "g" (code_size)
     :"%eax", "%ebx");
}

//function returns child PID, get exit #
int Wait(int *exit_status){
  int pid;
  asm("movl %1, %%eax; int $59; movl %%ebx, %0"
      : "=g" (pid)
      : "g" ((int) exit_status)
      : "%eax" , "%ebx");
  return pid;
}

//exit status # to return to parent
void Exit(int exit_status){
  asm("movl %0, %%eax; int $60"
      :
      :"g" (exit_status)
      :"%eax");
}
