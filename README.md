# 10-bit-assembler

The goal of this project was to create a compiler that translates source code, written in an imaginary assembly language \(with syntax symilar to MIPS Assembly language\) into machine language.

The machine is also imaginary and has a 10 bit architecture. The instructions are, of course, 10 bit long and the machine has 8 general purpose registers and 256 "cells" of memory \(each "cell" is 10 bits\).
This architecture supports the basic instructions you would expect (add, sub, jmp, etc...) and can use the following addressing methods:
  1. Immediate Addressing
  2. Direct Addressing
  3. Register Addressing
  4. Struct Access Addressing \(the language supports structs and this addressing mode is meant for accessing the fields of structs.\)

The program outputs 3 files written in base 32 numbers \(the base is also a bit weird. Basically, each number from 0 to 31 corresponds to a certain symbol. The full translation is shown below\):
  1. \(output_file\).as - the complete program with the data defined within it after the last istruction.
  2. \(output_file\).ent - contains the name and content of symbols defined as entries.
  3. \(output_file\).ext - contains the name of symbols defined as external and addresses of instructions referring to the symbol.

# usage
./assembler file1 file2... \(do not add file extension\)

# The 32 base
!,@,#,$,%,^,&,*,<,>,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v.

1 = !, 2 = @, and so so forth.
