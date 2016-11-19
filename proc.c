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

void TermCatCmd(char *cmd_str, int which){
   char obj_data[101], read_data[101];
   attr_t *attr_p;
   int fd;

   cmd_str+=4; //skip the first 4 letters for "cat "
   Fstat(cmd_str, obj_data);

   if(obj_data[0] == (char)0){
     TermOut("Fstat: no such file\n",which);
     return;
   }
   
   attr_p = (attr_t *)obj_data;  //cast data to attr ptr;
   if(A_ISDIR(attr_p->mode)) {  //if it is a directory, not a file
      TermOut("\aUsage: cat a file, not directory!\n", which);
      return;
   }

   fd = Fopen(cmd_str);

   //read loop
   while(1){
     Fread(fd,read_data);
     if(read_data[0] ==(char) 0) break;
     TermOut(read_data, which);
   }
   Fclose(fd);
}

void TermLsCmd(char *cmd_str, int which){
   char obj_data[101], read_data[101], a_str[101];
   attr_t *attr_p;
   int fd;

   cmd_str+=2; //skip the first 3 chars "ls "
   
   if(cmd_str[0] == (char)0) {  //if only ls is entered
     cmd_str[0] = '/';
     cmd_str[1] = (char)0;
   }else if(cmd_str[0] == ' ') 
      cmd_str++;

   Fstat(cmd_str, obj_data);

   if(obj_data[0] == (char)0){
     TermOut("Fstat: no such file\n",which);
     return;
   }
   
   attr_p = (attr_t *)obj_data;  //cast data to attr ptr;
   if(A_ISREG(attr_p->mode)) {  //if it is a file, not a directory
      Attr2Str(attr_p, a_str);   //convert obj data to "ls-able" string  
      TermOut(a_str, which);
      return;
   }

   //it is a directory
   fd = Fopen(cmd_str);
   //read loop
   while(1){
     Fread(fd, read_data);
     if (read_data[0] == (char)0) break;
     attr_p = (attr_t *)read_data;
     Attr2Str(attr_p, a_str);
     TermOut(a_str, which);
   }
   Fclose(fd);
}

void TermProc(){
   //char a_str[100];   // a handy string
   int i, baud_rate, divisor;
   int which;
   char login[101], passwd[101], cmd_str[101];

   which = GetPid() - 1;
   
   //initialize the port data structure
   term[which].out_q_sem = SemReq(); //request for out_q_sem

   for (i = 0; i < Q_SIZE; i++)  //boost the count to 20 
      SemPost(term[which].out_q_sem);
    
   term[which].in_q_sem = SemReq();  //request for in_q_sem 
   term[which].echo_flag = 1;
   term[which].out_flag = 1;

   //A.set baud rate 9600
   baud_rate = 9600;
   divisor = 115200/baud_rate;  //time period of each baud
   outportb(term[which].io_base + CFCR, CFCR_DLAB);  //CFCR_DLAB 0X80
   outportb(term[which].io_base + BAUDLO, LOBYTE(divisor));
   outportb(term[which].io_base + BAUDHI, HIBYTE(divisor));

   //B. set CFCR: 7-E-1 (7 data bits, even parity, 1 stop bit)
   outportb(term[which].io_base + CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
   outportb(term[which].io_base + IER, 0);

   //C. raise DTR, RTS of the serial port to start read/write
   outportb(term[which].io_base + MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   IO_DELAY();
   outportb(term[which].io_base + IER, IER_ERXRDY|IER_ETXRDY); //enable TX/RX events
   Sleep(50);

   MyBzero(cmd_str, sizeof(char)*100);

   for (i = 0; i < 24; i++)
     TermOut("\n", which);   //clear the screen

   while(1) {
     if(cons_kbhit() && cons_getchar() == 'b') breakpoint();
    
      while(1){ // loop for login
          TermOut("Team SKOS Login: ", which);
          TermIn(login, which);
          TermOut("Team SKOS Password: ", which);
          TermIn(passwd, which);
          
          if(1 == MyStrcmp(passwd, "pizza" , 5)){
            cons_printf("Term %d Login: %s Passwd: %s\n ", GetPid(), login, passwd);
           //MyStrcpy(a_str,"\t***Welcome! Commands are: ***\nls [file], cat <file>, logout\n");
            TermOut("**Welcome!**\n", which);
            break;
         }
         // if password didn't match
          TermOut("Illegal Login!\n", which);
      }
      while(1){ // loop for shell command
          TermOut("Team SKOS Shell> ", which);
          TermIn(cmd_str, which);

          if(cmd_str[0] ==(char) 0) continue; //if command is NULL, reloop
          else if(1 == MyStrcmp(cmd_str, "logout", 6) || 
                  1 == MyStrcmp(cmd_str, "000000", 6)) break;
          else if(1 == MyStrcmp(cmd_str, "ls", 2) ||
                  1 == MyStrcmp(cmd_str, "11", 2)) {
              // ls command
              TermLsCmd(cmd_str, which);
          }
          else if(1 == MyStrcmp(cmd_str, "cat", 3) || 
                  1 == MyStrcmp(cmd_str, "222", 3)){
              // cat command
              TermCatCmd(cmd_str, which);
          }
          else{
              TermOut("Command Not Known\n",which);
          }
      }
   }
}


void IdleProc() {
   int i;
   while(1){
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
