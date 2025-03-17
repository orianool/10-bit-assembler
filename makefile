assembler: assembler.o pre_assemble.o first_pass.o second_pass.o makefiles.o utils.o
	gcc -g -ansi -Wall -pedantic assembler.o pre_assemble.o first_pass.o second_pass.o makefiles.o utils.o -o assembler
assembler.o: assembler.c assembler.h pre_assemble.h first_pass.h second_pass.h makefiles.h utils.h  globals.h
	gcc -c -ansi -Wall -pedantic assembler.c -o assembler.o
pre_assemble.o: pre_assemble.c pre_assemble.h utils.h  globals.h
	gcc -c -ansi -Wall -pedantic pre_assemble.c -o pre_assemble.o
first_pass.o: first_pass.c first_pass.h utils.h globals.h
	gcc -c -ansi -Wall -pedantic first_pass.c -o first_pass.o
second_pass.o: second_pass.c second_pass.h utils.h globals.h
	gcc -c -ansi -Wall -pedantic second_pass.c -o second_pass.o
makefiles.o: makefiles.c makefiles.h utils.h globals.h
	gcc -c -ansi -Wall -pedantic makefiles.c -o makefiles.o
utils.o: utils.c utils.h globals.h
	gcc -c -ansi -Wall -pedantic utils.c -o utils.o
