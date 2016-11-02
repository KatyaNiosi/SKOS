// syscall.h

#ifndef _SYSCALL_H_
#define _SYSCALL_H_

int GetPid(void);  // no input, 1 return
void Sleep(int);   // 1 input, no return
int SemReq();
void SemWait(int);
void SemPost(int);
void TripTermIRQ();
#endif
