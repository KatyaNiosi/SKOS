// syscall.h

#ifndef _SYSCALL_H_
#define _SYSCALL_H_

int GetPid(void);  // no input, 1 return
void Sleep(int);   // 1 input, no return
int SemReq();
void SemWait(int);
void SemPost(int);
void TripTermIRQ();
void Fstat(char *cmd_str, char *obj_data);
int Fopen(char *cmd_str);
void Fread(int fd, char *read_data);
void Fclose(int fd);
void Fork(char *, int);
void SysWrite(char *);
int Wait(int *);
void Exit(int);
#endif
