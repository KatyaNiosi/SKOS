// main.h, 159

#ifndef _MAIN_H_
#define _MAIN_H_
#include "TF.h"

int main();
void InitKernelData();
void ProcScheduler();
void KernelMain(TF_t *);
void InitKernelControl();
void SetEntry(int, void ());

#endif
