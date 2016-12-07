# hello.s
#
# print a msg to terminal, exit with msg addr
# 
# Once this runs, it subtracts 4096 from ESP to get where its DRAM starts.
# $msg is given location 2G+x (a virtual address given by gcc link386).
# Subtract 2G from $msg to get the difference/offset x.
# Add x to starting DRAM addr, that's DRAM addr of msg.

.text                       # code segment
.global _start              # _start is public

_start:                     # _start is main()
   pushl  %esp
   popl   %edx               # ESP starts atop of this DRAM page
   
   subl  $0x1000, %edx       # minus 4KB --> start of page
   
   movl $msg, %ecx          # msg is x bytes into this page
   subl $0x80000000, %ecx   # subtract 2G (0x80000000), get x

   addl %edx, %ecx          # add x to start of page = msg addr
   pushl %ecx               # save a copy of msg addr into stack (pushl)

   movl %ecx, %eax
   int  $57                 # call SysWrite to output to terminal

   popl %eax                # get the saved copy of msg addr
   int  $60                 # call Exit(msg addr)

.data                       # data segment follows code segment in memory layout
msg:                        # msg
   .ascii "(SKOS) Hello!\n\r"
