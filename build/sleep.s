# sleep.s
#
# below as a demo code, it calls Sleep(300), change it to:
#    call Sleep for a period of seconds equal to its PID, and
#    call Exit with addr of its 1st inst as the exit code
#
# Enter "./build.pl sleep.s" to compile and convert to text sleep.bin
# build.pl does:
#    as --32 sleep.s -o sleep.o 
#    link386 -nostartfiles -userapp sleep.o -o sleep
# sleep.bin is to be included into file sys

# If curious about relative addr of identifiers below: as -a sleep.s

.text               # code segment 
.global _start      # _start is main()

_start:             # instructions begin

   int  $48         # call GetPid() 
   imul $100, %eax

   int  $49         # call Sleep(300)

   movl $_start, %eax   # exit status number
   int  $60         # call Exit

.data               # examples on how to declare data
x:
   .long 1        # integer, init 100
msg1:
   .ascii "Hello, World!\n\r"  # null-terminated string
msg2:                   # another string
   .ascii "Hey, Jude"
   .rept 20             # repeat 20 times
      .ascii "!"        # filled with exclamation marks
   .endr                # end of repeat
   .ascii "\n\r"

