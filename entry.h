// entry.h, 159

#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <spede/machine/pic.h>

#define TIMER_INTR  32
#define PRINTER_INTR 39
#define GETPID_INTR 48
#define SLEEP_INTR  49
#define SEMREQ_INTR 50
#define SEMWAIT_INTR 51
#define SEMPOST_INTR 52
#define TERM1_INTR 35
#define TERM2_INTR 36
#define FSTAT_INTR 53
#define FOPEN_INTR 54
#define FREAD_INTR 55
#define FCLOSE_INTR 56
#define SYSWRITE_INTR  57
#define FORK_INTR  58
#define WAIT_INTR  59
#define EXIT_INTR  60

#define KERNEL_CODE 0x08         // kernel's code segment
#define KERNEL_DATA 0x10         // kernel's data segment
#define KERNEL_STACK_SIZE 16384  // kernel's stack byte size

// ISR Entries
#ifndef ASSEMBLER

__BEGIN_DECLS

#include "TF.h"

extern void ProcLoader(TF_t *);     // code defined in entry.S
extern void TimerEntry();        // code defined in entry.S
extern void GetPidEntry();
extern void SleepEntry();
extern void SemReqEntry();
extern void SemWaitEntry();
extern void SemPostEntry();
extern void Term1Entry();
extern void Term2Entry();
extern void FstatEntry();
extern void FopenEntry();
extern void FreadEntry();
extern void FcloseEntry();
__END_DECLS

#endif

#endif
