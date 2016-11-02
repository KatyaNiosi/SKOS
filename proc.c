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

#define LOOP 1666667 

/*void PrinterProc(){
   int i, code;
   char key, *p;
   char hello[] = "Hello, World!\n\r";

   //cons_printf("************************************");
   //Request for a printer_sem 
   printer_sem = DeQ(&avail_sem_q);
   
   //initalize printer port (check printer power, cable, and paper)
   outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN); //CONTROL reg, Select interupt 
   code = inportb(LPT1_BASE+LPT_STATUS);    //Read Status
   for(i = 0; i < 50; i++) IO_DELAY();

   outportb(LPT1_BASE+LPT_CONTROL, PC_INIT | PC_SLCTIN | PC_IRQEN); // IRQ enable
//   for(i = 0; i < LOOP; i++) IO_DELAY();

   while(1){
     Sleep(100);
     if(!cons_kbhit()) continue;
       key = cons_getchar();
       switch(key){
          case 'b':
             breakpoint();
             break;
          case 'q':
             exit(0);
          default:
             cons_printf("Char: %c ", key);
       }

       if(key != '1' && key != '2') continue;
       
       p = hello;
       while(*p){
          outportb(LPT1_BASE+LPT_DATA, *p); // write char to port DATA reg
          code = inportb(LPT1_BASE+LPT_CONTROL); // read port COTROL reg
          
          outportb(LPT1_BASE+LPT_CONTROL, code | PC_STROBE); // write with STROBE 
          for(i = 0; i < 50; i++) IO_DELAY(); // Need delay
          
          outportb(LPT1_BASE+LPT_CONTROL, code); // send back original code 
         
          if(key == '1'){
                // busy-poll port status until ready
                for(i = 0; i < LOOP * 3; i++){
                    code = PS_ACK & inportb(LPT1_BASE + LPT_STATUS);
                    if(code == 0) break;
                    IO_DELAY();
                }
                
                // if times out cons_printf a msg and break loop
                if(i == LOOP * 3){
                    cons_printf(">>> Time out while printing a char!\n");   
                    break;
                }
          }
          if(key == '2'){
                // semaphore-wait on the printing semaphore
                if(sem[printer_sem].count > 0){
                    sem[printer_sem].count--;
                }else{
                SemWait(printer_sem);
                for (i = 0; i < 50; i++) IO_DELAY();
                }
          }
          p++;
       }
    }
}
*/
void TermIn(char *str){
  char ch, *p;
  p = str;

  while(1){
     SemWait(term[0].in_q_sem);
     ch = DeQ(&term[0].in_q);
     if(ch == 'r')
       break;
     *p = ch;
     p++;
   }
 *p = '\0';
}

void TermOut(char *str){
  char *p;
  p = str;

  while(*p){
    SemWait(term[0].out_q_sem);
    EnQ(*p, &term[0].out_q);
   
   if (*p == '\n'){
      SemWait(term[0].out_q_sem);
      EnQ('\r', &term[0].out_q);
    }
 
    TripTermIRQ();  //issue IRQ event to send to terminal
    p++;
  }
}

void TermProc(){
   char a_str[101];   // a handy string
   int i, baud_rate, divisor;

   term[0].out_q_sem = SemReq(); //request for out_q_sem

   for (i = 0; i < Q_SIZE; i++)  //boost the count to 20 
      SemPost(term[0].out_q_sem);
    
   term[0].in_q_sem = SemReq();  //request for in_q_sem 
   term[0].echo_flag = 1;
   term[0].out_flag = 1;

   //A.set baud rate 9600
   baud_rate = 9600;
   divisor = 115200/baud_rate;  //time period of each baud
   outportb(term[0].io_base + CFCR, CFCR_DLAB);  //CFCR_DLAB 0X80
   outportb(term[0].io_base + BAUDLO, LOBYTE(divisor));
   outportb(term[0].io_base + BAUDHI, HIBYTE(divisor));

   //B. set CFCR: 7-E-1 (7 data bits, even parity, 1 stop bit)
   outportb(term[0].io_base + CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
   outportb(term[0].io_base + IER, 0);

   //C. raise DTR, RTS of the serial port to start read/write
   outportb(term[0].io_base + MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   IO_DELAY();
   outportb(term[0].io_base + IER, IER_ERXRDY|IER_ETXRDY); //enable TX/RX events
   Sleep(50);

   while(1) {
      MyStrcpy(a_str,"\n\n\nHello, World! Team SKOS at Terminal 1\n");
      TermOut(a_str);
      MyStrcpy(a_str, "\nWhat is your name?\n\n");
      TermOut(a_str);
      TermIn(a_str);

      cons_printf("\nTerm 1 Name: %s", a_str);

      MyStrcpy(a_str, "\nHow is your farther?\n\n");
      TermOut(a_str);
      TermIn(a_str);

      cons_printf("Term 1 Father Status: %s",a_str);
      
      MyStrcpy(a_str, "\nCan you elaborate that?\n\n");
      TermOut(a_str);
      TermIn(a_str);

      cons_printf("\nTerm 1 Replied: %s", a_str);
   }
 }

void IdleProc() {
   int i;
   while(1){
     if(cons_kbhit() && cons_getchar() == 'b') breakpoint();
     cons_printf("0..");
     for(i = 0; i < 1666667; i++) IO_DELAY();
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
