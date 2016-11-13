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

#define LOOP 166666 

void TermIn(char *str, int which){
  char ch, *p;
  p = str;
  
  while(1){
     SemWait(term[which].in_q_sem);
     ch = DeQ(&term[which].in_q);
     if(ch == '\r')
       break;
     *p++ = ch;
  }
  *p = '\0';
}

void TermOut(char *str, int which){
  char *p;
  p = str;

  while(*p){
    SemWait(term[which].out_q_sem);
    EnQ(*p, &term[which].out_q);
   
   if (*p == '\n'){
      SemWait(term[which].out_q_sem);
      EnQ('\r', &term[which].out_q);
    }
 
    TripTermIRQ();  //issue IRQ event to send to terminal
    p++;
  }
}

void TermProc(){
   char a_str[100];   // a handy string
   int i, baud_rate, divisor, which;
   int local, remote;
   char login[101], passwd[101], cmd_str[101];

   local = GetPid() - 1;
   remote = (local + 1)%2;   //0->1   1->0

   //initialize the port data structure
   term[local].out_q_sem = SemReq(); //request for out_q_sem

   for (i = 0; i < Q_SIZE; i++)  //boost the count to 20 
      SemPost(term[local].out_q_sem);
    
   term[local].in_q_sem = SemReq();  //request for in_q_sem 
   term[local].echo_flag = 1;
   term[local].out_flag = 1;

   //A.set baud rate 9600
   baud_rate = 9600;
   divisor = 115200/baud_rate;  //time period of each baud
   outportb(term[local].io_base + CFCR, CFCR_DLAB);  //CFCR_DLAB 0X80
   outportb(term[local].io_base + BAUDLO, LOBYTE(divisor));
   outportb(term[local].io_base + BAUDHI, HIBYTE(divisor));

   //B. set CFCR: 7-E-1 (7 data bits, even parity, 1 stop bit)
   outportb(term[local].io_base + CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
   outportb(term[local].io_base + IER, 0);

   //C. raise DTR, RTS of the serial port to start read/write
   outportb(term[local].io_base + MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   IO_DELAY();
   outportb(term[local].io_base + IER, IER_ERXRDY|IER_ETXRDY); //enable TX/RX events
   Sleep(50);

   for (i = 0; i < 24; i++)
     TermOut("\n", local);   //clear the screen

   while(1) {
      TermIn(a_str, local);
      cons_printf("Term %d entered: %s\n", GetPid(), a_str); 
      TermOut("(Remote)", remote);
      TermOut(a_str, remote);
      TermOut("\n", remote);

    
      while(1){ // loop for login
          cons_printf("Enter Username");
          cons_printf("Enter Password");
          int len = MyStrlen(a_str);
          if(1 == MyStrcmp(a_str, "pizza" , len)){
            break;
          }
      }
      while(1){ // loop for shell command
          cons_printf("Enter Command");
          TermIn(a_str, local);
          which = remote;
          int len = MyStrlen(a_str);
          if(a_str == NULL) continue;
          else if(1 == MyStrcmp(a_str, "logout", len)) break;
          else if(1 == MyStrcmp(a_str, "ls", len)){
              // ls command
              TermLsCmd(a_str, which);
          }
          else if(1 == MyStrcmp(a_str, "cat", len)){
              // cat command
              TermCatCmd(a_str, which);
          }
          else{
              cons_printf("Command Not Known");
          }
      }
   }
}

void TermCatCmd(char *cmd_str, int which){

}

void TermLsCmd(char *cmd_str, int which){

}

void IdleProc() {
   int i;
   while(1){
     if(cons_kbhit() && cons_getchar() == 'b') breakpoint();
     if(cons_kbhit() && cons_getchar() == 'q') exit(0);
     cons_printf("0..");
     for(i = 0; i < 166666; i++) IO_DELAY();
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
