// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

#include "typedef.h" // needed to define func_ptr_t

void NewProcISR(int, func_ptr_t);
void KillProcISR();
void TimerISR();
void SleepISR();
void GetPidISR();
void SemReqISR();
void SemWaitISR(int);
void SemPostISR(int);
void TermISR();
void FstatISR();
void FopenISR();
void FcloseISR();
void FreadISR();

#endif
