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
   ????                     # ESP starts atop of this DRAM page
   ????                     # minus 4KB --> start of page

   ????                     # msg is x bytes into this page
   ????                     # subtract 2G (0x80000000), get x

   ????                     # add x to start of page = msg addr
   ????                     # save a copy of msg addr into stack (pushl)

   ????                     # call SysWrite to output to terminal

   ????                     # get the saved copy of msg addr
   ????                     # call Exit(msg addr)

.data                       # data segment follows code segment in memory layout
msg:                        # msg
   .ascii "(Team Name): Hello! Good things need no arguments, bad things worth no arguments.\n\r"

