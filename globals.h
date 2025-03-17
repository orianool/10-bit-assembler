
#ifndef MAMAN14_GLOBALS_H
#define MAMAN14_GLOBALS_H

#define MAX_LINE 81
#define NULL_COMMAND 16
#define NULL_INSTRUCTION 5
#define MAX_INT 128
#define MAX_FILENAME_LEN 2024
#define MEM_DISPLACEMENT 100
#define MEM_LIMIT 255


/* linked list of labels */
typedef struct label_table_node{
    char *name;
    unsigned int address;
    int data_count; /* amount of data used by label */
    struct label_table_node *next;
} Label_table_node;

/* data contained in a "word" */
typedef struct statement {
    unsigned int code : 10;
}Statement;

/* linked list "words" */
typedef struct binary_table_node {
    unsigned int mem_address;
    Statement statement;
    struct binary_table_node *next;
} Binary_table_node;

/* pointer to function analyzing commands \ instructions */
typedef int (*func_ptr)(char *, Binary_table_node *);

/* addressing type of operands */
typedef struct address {
    unsigned int val : 2; /* index of type */
    unsigned int words : 2; /* amount of word needed by operand */
} Address;

/* Register */
typedef struct reg {
    char* name;
    unsigned int reg_num : 3; /* index of Register */
} Reg;

/* Command */
typedef struct command {
    char* name;
    unsigned int code : 4; /* code of command */
    func_ptr function; /* pointer to function to analyze the command */
} Command;

/* Instruction */
typedef struct instruction {
    char* name;
    func_ptr function; /* pointer to function to analyze the instruction */
} Instruction;

/* linked list of entries */
typedef struct entry_node {
    char* name;
    unsigned int mem_address; /* memory address of entry */
    struct entry_node *next;
} Entry_node;

/* linked list of externals */
typedef struct extern_node {
    char* name;
    struct extern_node *next;
} Extern_node;

/* linked list of call to an external */
typedef struct extern_call_node {
    char* name;
    unsigned int mem_address; /* memory address of statement calling the extern */
    struct extern_call_node *next;
} Extern_call_node;

#endif
