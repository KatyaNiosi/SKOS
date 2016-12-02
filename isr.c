// isr.c, 159

#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "tools.h"
#include "extern.h"
#include "proc.h"


// Search our (fixed size) table of file descriptors. returns fd_array[] index
// if an unused entry is found, else -1 if all in use. if avail, then all
// fields are initialized.
// check ownership of fd and the fd is valid within range
int CanAccessFD( int fd, int owner ) {
   if( VALID_FD_RANGE(fd) && fd_array[fd].owner == owner) return 0;
   return -1;     // not good
}

// go searching through a single dir for a name match. use MyStrcmp()
// for case-insensitive compare. use '/' to separate directory components
// if more after '/' and we matched a dir, recurse down there
// RETURN: ptr to dir entry if found, else 0
// once any dir matched, don't return name which dir was matched
dir_t *FindNameSub( char *name, dir_t *this_dir ) {
   dir_t *dir_p = this_dir;
   int len = MyStrlen( name );
   char *p;

// if name is '.../...,' we decend into subdir
   if( ( p = strchr( name, '/' ) ) != 0) len = p - name;  // p = to where / is (high mem)

   for( ; dir_p->name; dir_p++ ) {
//      if((unsigned int)dir_p->name > 0xdfffff) return 0; // tmp bug-fix patch

      if( 1 == MyStrcmp( name, dir_p->name, len ) ) {
         if( p && p[1] != 0 ) { // not ending with name, it's "name/..."
// user is trying for a sub-dir. if there are more components, make sure this
// is a dir. if name ends with "/" we don't check. thus "hello.html/" is legal
            while( *p == '/' ) {
               p++;                           // skipping trailing /'s in name
               if( '\0' == *p ) return dir_p; // name "xxx/////" is actually legal
            }

// altho name given is "xxx/yyy," xxx is not a directory
            if(dir_p->mode != MODE_DIR) return 0; // bug-fix patch for "cat h/in"

            name = p;
            return FindNameSub( name, (dir_t *)dir_p->data );
         }
         return dir_p;
      }
   }

   return 0;   // no match found
}

int AllocFD( int owner ) {
   int i;

   for(i=0; i<MAX_FD; i++) {
      if( -1 == fd_array[i].owner ) {
         fd_array[i].owner = owner;
         fd_array[i].offset = 0;
         fd_array[i].item = 0;     // NULL is (void *)0, spede/stdlib.h

         return i;
      }
   }

   return -1;   // no free file descriptors
}

void FreeFD( int fd ) {  // mark a file descriptor as now free
   fd_array[fd].owner = -1;
}

dir_t *FindName( char *name ) {
   dir_t *starting;

// assume every path relative to root directory. Eventually, the user
// context will contain a "current working directory" and we can possibly
// start our search there
   if( name[0] == '/' ) {
      starting = root_dir;

      while( name[0] == '/' ) name++;

      if( name[0] == 0 ) return root_dir; // client asked for "/"
   } else {
// path is relative, so start off at CWD for this process
// but we don't have env var CWD, so just use root as well
      starting = root_dir; // should be what env var CWD is
   }

   if( name[0] == 0 ) return 0;

   return FindNameSub( name, starting );
}

// copy what dir_p points to (dir_t) to what attr_p points to (attr_t)
void Dir2Attr( dir_t *dir_p, attr_t *attr_p ) {
   attr_p->dev = run_pid;            // run_pid manages this i-node

   attr_p->inode = dir_p->inode;
   attr_p->mode = dir_p->mode;
   attr_p->nlink = ( A_ISDIR( attr_p->mode ) ) + 1;
   attr_p->size = dir_p->size;
   attr_p->data = dir_p->data;
}


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

void TermOutHandler(int which){
  char ch = 0;

  if(term[which].echo_q.size > 0)
    ch = DeQ(&term[which].echo_q);
  else{
    if(term[which].out_q.size > 0){
      ch = DeQ(&term[which].out_q);
      SemPostISR(term[which].out_q_sem);
    }
  }

  if (ch != '\0'){
    outportb(term[which].io_base + DATA, ch);
    term[which].out_flag = 0;
   }else{
     term[which].out_flag = 1;
   }
 }

 void TermInHandler(int which){
   char ch;

   //use 127 to mask out msb (get normal 7-bit ASCII range)
   ch = inportb(term[which].io_base + DATA) & 0x7F; //mask 0111 1111

   EnQ(ch, &term[which].in_q);
   SemPostISR(term[which].in_q_sem);

   if (ch == '\r'){
     EnQ('\r', &term[which].echo_q);
     EnQ('\n', &term[which].echo_q);
   } else{
      if(term[which].echo_flag == 1)
         EnQ(ch, &term[which].echo_q);
    }
 }

void TermISR() {
  int code, which;
  
  for(which = 0; which < 3; which++){
     code = inportb(term[which].io_base + IIR);

     switch(code){
       case IIR_TXRDY:
         TermOutHandler(which);
         break;

       case IIR_RXRDY:
         TermInHandler(which);
         break;
     }
    if(term[which].out_flag == 1)
      TermOutHandler(which);
  }
} 

void SemReqISR(){
   
   int new_sem = DeQ(&avail_sem_q);

   // If no semaphore is left return -1 
   if (new_sem == -1){
     pcb[run_pid].TF_p->eax = -1;
     return;
   }
   // Return to process via its trapframe
   pcb[run_pid].TF_p->eax = new_sem;

   // Semaphore is to be initalized to zero bytes    
   MyBzero((char *)&sem[new_sem], sizeof(sem_t));
   
}

void SemWaitISR(int sem_id){
   int count = sem[sem_id].count;
  
   if(count > 0){
      sem[sem_id].count--;
   }
   if(count == 0){
      EnQ(run_pid, &(sem[sem_id].wait_q));
      pcb[run_pid].state = WAIT;
      run_pid = -1;    
   }
}

void SemPostISR(int sem_id){
   int wait_pid = DeQ(&(sem[sem_id].wait_q));
   if(wait_pid == -1){
      sem[sem_id].count++;
   }
   else{
      EnQ(wait_pid, &ready_q);    // queue its PID (run_pid) to ready_q
      pcb[wait_pid].state = READY;
   }
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

// filesys_isr.txt, code to extend isr.c in phase 6 filesys
///////////////////////////////////////////////////// phase 6 filesys
void FstatISR(void) {
   char *name, *obj_data;
   attr_t *attr_p;
   dir_t *dir_p;

   name = (char *)pcb[run_pid].TF_p->eax;
   obj_data = (char *)pcb[run_pid].TF_p->ebx;

   dir_p = FindName( name );

   if( ! dir_p ) {      // dir_p == 0, not found
      obj_data[0] = 0;  // null terminated, not found, return
      return;
   }

   attr_p = (attr_t *)obj_data;
   Dir2Attr( dir_p, attr_p ); // copy what dir_p points to to where attr_p points to

// should include filename (add 1 to length for null char)
   MyMemcpy( (char *)( attr_p + 1 ), dir_p->name, MyStrlen( dir_p->name ) + 1 );
}

void FopenISR(void) {
   char *name;
   int fd;
   dir_t *dir_p;

   name = (char *)pcb[run_pid].TF_p->eax;

   fd = AllocFD( run_pid );  // run_pid is owner of fd allocated

   if( fd == -1 ) {
      cons_printf( "FopenISR: no more File Descriptor!\n" );
      pcb[run_pid].TF_p->ebx = -1;
      return;
   }

   dir_p = FindName( name );
   if( ! dir_p ) {
      cons_printf( "FopenISR: name not found!\n" );
      pcb[run_pid].TF_p->ebx = -1;
      return;
   }

   fd_array[fd].item = dir_p;    // dir_p is the name
   pcb[run_pid].TF_p->ebx = fd;  // process gets this to future read
}

void FcloseISR(void) {
   int fd, result;

   fd = pcb[run_pid].TF_p->eax;

   result = CanAccessFD( fd, run_pid ); // check if run_pid owns this FD
   if( result == 0 ) FreeFD( fd );
   else  cons_printf( "FcloseISR: cannot close FD!\n" );
}

// Copy bytes from file into user's buffer. Returns actual count of bytes
// transferred. Read from fd_array[fd].offset (initially given 0) for
// buff_size in bytes, and record the offset. may reach EOF though...
void FreadISR(void) {
   int fd, result, remaining;
   char *read_data;
   dir_t *lp_dir;

   fd = pcb[run_pid].TF_p->eax;
   read_data = (char *)pcb[run_pid].TF_p->ebx;

   result = CanAccessFD( fd, run_pid ); // check if run_pid owns this FD

   if( result != 0 ) {
      cons_printf( "FreadISR: cannot read from FD!\n" );
      read_data[0] = 0;  // null-terminate it
   }

   lp_dir = fd_array[fd].item;

   if( A_ISDIR(lp_dir->mode ) ) {  // it's a dir
// if reading directory, return attr_t structure followed by obj name.
// a chunk returned per read. `offset' is index into root_dir[] table.
      dir_t *this_dir = lp_dir;
      attr_t *attr_p = (attr_t *)read_data;
      dir_t *dir_p;

      if( 101 < sizeof( *attr_p ) + 2) {
         cons_printf( "FreadISR: read buffer size too small!\n" );
         read_data[0] = 0;  // null-terminate it
         return;
      }

// use current dir, advance to next dir for next time when called
      do {
         dir_p = ( (dir_t *)this_dir->data );
         dir_p += fd_array[fd].offset ;

         if( dir_p->inode == END_DIR_INODE ) {
            read_data[0] = 0;  // EOF, null-terminate it
            return;
         }
         fd_array[fd].offset++;   // advance
      } while( dir_p->name == 0 );

// MyBzero() fills buff with 0's, necessary to clean buff
// since Dir2Attr may not completely overwrite whole buff...
      MyBzero( read_data, 101 );
      Dir2Attr( dir_p, attr_p );

// copy obj name after attr_t, add 1 to length for null
      MyMemcpy((char *)( attr_p + 1 ), dir_p->name, MyStrlen( dir_p->name ) + 1);

   } else {  // a file, not dir
// compute max # of bytes can transfer then MyMemcpy()
      remaining = lp_dir->size - fd_array[fd].offset;

      if( remaining == 0 ) {
         read_data[0] = 0;  // EOF, 1st char becomes '\0'
         return;
      }

      MyBzero( read_data, 101 );  // null termination for any part of file read

      result = remaining<100?remaining:100; // -1 saving is for last null

      MyMemcpy( read_data, &lp_dir->data[ fd_array[ fd ].offset ], result );

      fd_array[fd].offset += result;  // advance our "current" ptr
   }
}

void ForkISR(void){
  int code_addr, code_size, pid, page_num;

  //A. allocate a DRAM page
  for (page_num = 0; page_num < PAGE_NUM; page_num++){
    if(page_info[page_num].owner == -1)  // found available page
      break;
    if (page_num == PAGE_NUM - 1)
      cons_printf("Kernel Panic: no free DRAM space left!\n");
      return;
  }
  //B. allocate PID
  pid = DeQ(&avail_q);
  if(pid == -1){
    cons_printf("Kernel panic: no more processes!\n");
    return;
  }

  //C. set DRAM page usage info 
  // ?????????
  
  //D. clear DRAM page 
  MyBzero((char *)&page_info[page_num], sizeof(page_info_t));

  //E. copy "a.out" image into the allocated DRAM page
  code_addr = pcb[run_pid].TF_p->eax;
  code_size = pcb[run_pid].TF_p->ebx;
  MyMemcpy((char *)page_info[page_num].addr, (char *)code_addr, code_size);
  
 //F. clear PCB
 // need to finish this isr
}

void ExitISR(void){
  int idParent, childExitNum, i;
  int *parentExitNum;
  idParent = pcb[run_pid].ppid;
  if(idParent.state != WAIT4CHILD){
      pcb[run_pid].state = ZOMBIE;
      run_pid = -1;
      return;
  }

  pcb[idParent].state = READY;
  EnQ(idParent, ready_q);

  childExitNum = pcb[run_pid].TF_p->eax;
  pcb[idParent].TF_p->ebx = run_pid;
  parentExitNum = (int *)pcb[idParent].TF_p->eax;
  *parentExitNum = childExitNum;
  
  for(i = 0; i < PAGE_NUM; i++){
    if(page_info[i].owner == run_pid){
        page_info[i].owner = -1;
        MyBzero((char *)page[i].addr, 4096);
    }
  }
  MyBzero((char *)&pcb[run_pid], sizeof(pcb_t));
  EnQ(run_pid, avail_q);
  run_pid = -1;
}

void SysWriteISR(void){
  int which;
  char *p;
  which = run_pid - 1;
  p = (char *)pcb[which].TF_q->eax;
  MyStrcpy(term[which].stdout_q, p);
  TermOutHandler(which);
}
