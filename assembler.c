
/* assembler: converts .as files (assembly) in to machine code ( in base 32) */

#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "pre_assemble.h"
#include "globals.h"
#include "first_pass.h"
#include "second_pass.h"
#include "makefiles.h"

Reg registers[] = {
	{"r0", 0},
	{"r1", 1},
	{"r2", 2},
	{"r3", 3},
	{"r4", 4},
	{"r5", 5},
	{"r6", 6},
	{"r7", 7},
	{NULL, 0}
};

Command commands[] = {
	{"mov", 0, mov},
	{"cmp", 1, cmp},
	{"add", 2, add},
	{"sub", 3, sub},
	{"not", 4, not},
	{"clr", 5, clr},
	{"lea", 6, lea},
	{"inc", 7, inc},
	{"dec", 8, dec},
	{"jmp", 9, jmp},
	{"bne", 10, bne},
	{"get", 11, get},
	{"prn", 12, prn},
	{"jsr", 13, jsr},
	{"rts", 14, rts},
	{"hlt", 15, hlt},
	{NULL, 0, NULL}
};

Instruction instructions[] = {
        {".data", f_data},
        {".string", f_string},
        {".struct", f_struct},
        {".entry", f_entry},
        {".extern", f_extern},
        {NULL, NULL}
};

Address address_type[] = {
        {0, 1},
        {1, 1},
        {2, 2},
        {3, 1}
};

Binary_table_node *binaryTableNode = NULL;
Label_table_node *label_table = NULL;
Entry_node *entries_table = NULL;
Extern_node *extern_table = NULL;
Extern_call_node *extern_call_list = NULL;

unsigned int DC, IC;
static char file_name[FILENAME_MAX]; /* .as extensions is included in FILENAME_MAX */

/* main: process command line input */
int main(int argc, char *argv[])
{
    int i, status;
    FILE *source;
    char *prog; /* for error handling */

    char *as_extension = ".as";
    prog = argv[0];
    /* checking for correct input */
    if(argc == 1) {
        fprintf(stderr, "Error: Missing arguments\nUsage: ./assembler file1 file2...\n");
        return 0;
    }

    /* opening files and starting translation */
    for(i = 1; i < argc; i++) {
        strcpy(file_name, argv[i]);
        strcat(file_name, as_extension);
        if((source = fopen(file_name, "r")) == NULL){
            fprintf(stderr,"%s: Can't open %s\n", prog, argv[1]);
            continue;
        }
        DC = IC = 0; /* resets counters */
        status = build(source, argv[i]);
        if(!status)
            printf("%s: failed\n", argv[i]);
        else
            printf("%s: succeeded\n", argv[i]);
        fclose(source);
    }
    return 0;
}

/* build: controls the translation of an .as file from assembly to machine code.
   FILE *fp - pointer to file to be translated
   char* f_name - name of files created during translation
   return - 1 for succeeded, 0 if failed */
int build(FILE *fp, char* f_name) {
    FILE * pre_assembled_file;

    /* creates a new file. This file will be the file that will be compiled */
    pre_assembled_file = fopen("preprocessed_file", "w+");
    if(!pre_assemble(fp ,pre_assembled_file)) { /* pre_assemble failed */
        printf("%s: pre_assemble failure, terminated\n", f_name);
        return 0;
    }
    rewind(pre_assembled_file);
    if(first_pass(pre_assembled_file)) {
        rewind(pre_assembled_file);
        if(second_pass(pre_assembled_file)) {
            if(makefiles(f_name)) {
                return 1;
            }
            printf("\"%s: makefiles failed, assembler terminated\n", f_name);
            remove("preprocessed_file");
            return 0;
        }
        printf("%s: second_pass failed, assembler terminated\n", f_name);
        remove("preprocessed_file");
        return 0;
    }
    printf("%s: first_pass failed, assembler terminated\n", f_name);
    remove("preprocessed_file");
    return 0;
}