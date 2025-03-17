
#include <stdio.h>
#include <string.h>
#include "second_pass.h"
#include "utils.h"
#include "globals.h"

static Statement SOURCE_OP_MASK, DESTINATION_OP_MASK, MASK_RES;
static unsigned int line_num, pass_status;

extern char line[]; /* will be used by functions in first_pass during file analysis. */
extern char word[]; /* will be used by functions in first_pass during file analysis. */
extern Binary_table_node *binaryTableNode;
extern Label_table_node *label_table;
extern Entry_node *entries_table;
extern Extern_node *extern_table;
extern Extern_call_node *extern_call_list;
extern unsigned int IC;
unsigned int ILC; /* command \ instruction lines counter - will be used by makefiles */

char *type_one[] = {
        "mov",
        "cmp",
        "add",
        "sub",
        "lea",
        NULL
};

char *type_two[] = {
        "not",
        "clr",
        "inc",
        "jmp",
        "bne",
        "get",
        "prn",
        "jsr",
        NULL
};

char *type_three[] = {
        "rts",
        "hlt",
        NULL
};

/* second_pass: iterates over source file and completes the binary tables
   FILE *source - file being translated
   return - 1 if succeeded, 0 if failed */
int second_pass(FILE *source) {
    int line_index, type, instruction_index;
    Label_table_node *label;
    Entry_node *entry;
    
    IC = MEM_DISPLACEMENT;
    ILC = 0;
    pass_status = 0;


    extern_call_list = NULL;
    line_num = 1;
    SOURCE_OP_MASK.code = 48;
    DESTINATION_OP_MASK.code = 12;

    while(get_line(source) != NULL) {

        if(is_comment_empty_line(line)) {
            line_num++;
            continue;
        }

        line_index = get_word_abs(line, 0, NULL);
        if(is_label(word)) {
            word[strlen(word) - 1] = '\0'; /* removes ':' */
            for(label = label_table; strcmp(label->name,word); label = label->next) /* safe because word is definitely a label */
                ;
            line_index = get_word_abs(line, line_index, NULL); /* get command / instruction */
        }
        if((instruction_index = is_instruction(word)) != NULL_INSTRUCTION) {
            if(instruction_index <= 2) { /* if instruction is .data / .string / .struct */
                IC += label->data_count;
            }
            else if(instruction_index == 3) { /* instruction is .entry */
                line_index = get_word_abs(line, line_index, NULL); /* get parameter */
                for(label = label_table; label != NULL && strcmp(word, label->name); label = label->next)
                    ;
                if(label == NULL){
                    printf("ERROR - line %u: \"%s\" is not defined\n", line_num, word);
                    pass_status++;
                }
                /* find entry in entries_table and copies address from label_table */
                for(entry = entries_table; strcmp(word, entry->name); entry = entry->next) /* safe because word is definitely an entry */
                    ;
                entry->mem_address = label->address;
            }
        }
        else { /* if word is not instruction then word has to be command */
            type = get_type(word);
            switch (type) { /* no need for default */
                case 1:
                    IC += f_type_one(get_binary(IC), line_index);
                    break;
                case 2:
                    IC += f_type_two(get_binary(IC), line_index);
                    break;
                case 3:
                    IC += f_type_three(get_binary(IC), line_index);
                    break;
            }
        }
        ILC++;
        line_num++;
    }
    free_label_list(label_table); /* no more use for label table */
    free_extern_list(extern_table); /* no more use for extern table. make file uses different list */
    if(pass_status)
        return 0;
    return 1;
}

/* get_binary: gets the binary code from specified location in memory
   unsigned int pos - position in memory
   returns - Statement containing the binary code in shape of a bit field of size 10 */
Statement get_binary(unsigned int pos) {
    Statement res;
    int i;
    Binary_table_node *current;

    pos -= MEM_DISPLACEMENT;
    current = binaryTableNode;

    for(i = 0; i < pos; i++) /* get to binary node in pos */
        current = current->next;

    res.code = current->statement.code;

    return res;
}

/* get_type: gets the type of a command
   char *str - the command
   returns - 1 if command gets two operands, 2 if command gets onr operand, 3 if command gets no operands */
int get_type(char *str) {
    int i;
    char *tmp;

    for(i = 0, tmp = type_one[0]; tmp != NULL; tmp = type_one[++i]){
        if(!strcmp(tmp, word))
            return 1;
    }
    for(i = 0, tmp = type_two[0]; tmp != NULL; tmp = type_two[++i]){
        if(!strcmp(tmp, word))
            return 2;
    }
    for(i = 0, tmp = type_three[0]; tmp != NULL; tmp = type_three[++i]){
        if(!strcmp(tmp, word))
            return 3;
    }
    return 0;
}

/* f_type_one: analyzes the amount of data used by a command  that gets two operands.
   Statement statement - command line in binary.
   int line_index - index of the command in the command line
   return - amount memory used by command */
int f_type_one(Statement statement, int line_index) {
    int res, REG_FLAG;
    Statement tmp;
    Label_table_node *label;

    label = label_table;
    REG_FLAG = res = 0;

    MASK_RES.code = statement.code & SOURCE_OP_MASK.code;
    switch (MASK_RES.code) { /* no need for default */
        case 0: /* address type: 00 */
            res++;
            line_index = get_word_abs(line, line_index, NULL); /* get parameter from after command name */
            break;
        case 16: /* address type: 01 */
            res++;

            line_index = get_word_abs(line, line_index, NULL); /* get parameter from after command name */

            if(is_extern(word)) {
                tmp.code = 1; /* adds ARE (External) */
                replace_code(IC + 1, tmp);
                add_extern_call_node(word, IC + 1);
            }
            else {
                while(label != NULL && strcmp(word, label->name)) { /* find label memory */
                    label = label->next;
                }
                if(label != NULL) {
                    tmp.code = label->address << 2;
                    tmp.code += 2; /* adds ARE (Relocatable) */
                    replace_code(IC + 1, tmp);
                    line_index = get_word_delim(line, line_index, ','); /* to move past struct field */
                    break;
                }
                printf("ERROR - line %u: \"%s\" is not defined\n", line_num, word);
                pass_status++;
            }
            break;
        case 32: /* address type: 10 */
            res += 2;
            line_index = get_word_delim(line, line_index, '.'); /* get struct name */
            if(is_extern(word)) {
                tmp.code = 1; /* adds ARE (External) */
                replace_code(IC + 1, tmp);
                add_extern_call_node(word, IC + 1);
            }
            else {
                while(label != NULL && strcmp(word, label->name)) { /* find label memory */
                    label = label->next;
                }
                if(label != NULL) {
                    tmp.code = label->address << 2;
                    tmp.code += 2; /* adds ARE (Relocatable) */
                    replace_code(IC + 1, tmp);
                    line_index = get_word_delim(line, line_index, ','); /* to move past struct field */
                    break;
                }
                printf("ERROR - line %u: \"%s\" is not defined\n", line_num, word);
                pass_status++;
            }
            break;
        case 48: /* address type: 11 */
            res++;
            line_index = get_word_abs(line, line_index, NULL); /* get parameter from after command name */
            REG_FLAG = 1;
            break;
    } /* end of source analysis */

    MASK_RES.code = statement.code & DESTINATION_OP_MASK.code;
    label = label_table; /* resets label to head of list */
    switch (MASK_RES.code) { /* no need for default */
        case 0: /* address type: 00 */
            res++;
            line_index = get_word_abs(line, line_index, NULL); /* get parameter from after command name */
            break;
        case 4: /* address type: 01 */
            res++;

            line_index = get_word_abs(line, line_index, NULL); /* get parameter from after command name */

            if(is_extern(word)) {
                tmp.code = 1; /* adds ARE (External) */
                replace_code(IC + res, tmp);
                add_extern_call_node(word, IC + res);
            }
            else {
                while(label != NULL && strcmp(word, label->name)) { /* find label memory */
                    label = label->next;
                }
                if(label != NULL) {
                    tmp.code = label->address << 2;
                    tmp.code += 2; /* adds ARE (Relocatable) */
                    replace_code(IC + res, tmp);
                    line_index = get_word_delim(line, line_index, ','); /* to move past struct field */
                    break;
                }
                printf("ERROR - line %u: \"%s\" is not defined\n", line_num, word);
                pass_status++;
            }
            break;
        case 8: /* address type: 10 */
            res += 2;
            line_index = get_word_delim(word,0 , '.'); /* get struct name */
            if(is_extern(word)) {
                tmp.code = 1; /* adds ARE (External) */
                replace_code(IC + res, tmp);
                add_extern_call_node(word, IC + res);
            }
            else {
                while(label != NULL && strcmp(word, label->name)) { /* find label memory */
                    label = label->next;
                }
                if(label != NULL) {
                    tmp.code = label->address << 2;
                    tmp.code += 2; /* adds ARE (Relocatable) */
                    replace_code(IC + res, tmp);
                    line_index = get_word_delim(line, line_index, ','); /* to move past struct field */
                    break;
                }
                printf("ERROR - line %u: \"%s\" is not defined\n", line_num, word);
                pass_status++;
            }
            break;
        case 12: /* address type: 11 */
            if(!REG_FLAG)
                res++;
            line_index = get_word_abs(line, line_index, NULL); /* get parameter from after command name */
            break;
    } /* end of destination analysis */
    return ++res; /* adds 1 so when adding res to IC, IC will be the address of next line */
}

/* f_type_two: analyzes the amount of data used by a command  that gets one operand.
   Statement statement - command line in binary.
   int line_index - index of the command in the command line
   return - amount memory used by command */
int f_type_two(Statement statement, int line_index) {
    int res;
    Statement tmp;
    Label_table_node *label;

    label = label_table;
    res = 0;

    MASK_RES.code = statement.code & DESTINATION_OP_MASK.code;
    switch (MASK_RES.code) { /* no need for default */
        case 0: /* address type: 00 */
            res++;
            line_index = get_word_abs(line, line_index, NULL); /* get parameter from after command name */
            break;
        case 4: /* address type: 01 */
            res++;

            line_index = get_word_abs(line, line_index, NULL); /* get parameter from after command name */

            if(is_extern(word)) {
                tmp.code = 1; /* adds ARE (External) */
                replace_code(IC + 1, tmp);
                add_extern_call_node(word, IC + 1);
            }
            else {
                while(label != NULL && strcmp(word, label->name)) { /* find label memory */
                    label = label->next;
                }
                if(label != NULL) {
                    tmp.code = label->address << 2;
                    tmp.code += 2; /* adds ARE (Relocatable) */
                    replace_code(IC + 1, tmp);
                    line_index = get_word_delim(line, line_index, ','); /* to move past struct field */
                    break;
                }
                printf("ERROR - line %u: \"%s\" is not defined\n", line_num, word);
                pass_status++;
            }
            break;
        case 8: /* address type: 10 */
            res += 2;
            line_index = get_word_delim(word,0 , '.'); /* get struct name */
            if(is_extern(word)) {
                tmp.code = 1; /* adds ARE (External) */
                replace_code(IC + 1, tmp);
                add_extern_call_node(word, IC + 1);
            }
            else {
                while(label != NULL) { /* find label memory */
                    if(!strcmp(word, label->name)) {
                        tmp.code = label->address << 2;
                        tmp.code += 2; /* adds ARE (Relocatable) */
                        replace_code(IC + 1, tmp);
                    }
                    label = label->next;
                }
            }
            break;
        case 12: /* address type: 11 */
            res++;
            line_index = get_word_abs(line, line_index, NULL); /* get parameter from after command name */
            break;
    } /* end of destination analysis */
    return ++res; /* adds 1 so when adding res to IC, IC will be the address of next line */
}

/* f_type_two: analyzes the amount of data used by a command  that gets no operands.
   Statement statement - command line in binary.
   int line_index - index of the command in the command line
   return - amount memory used by command (always 1) */
int f_type_three(Statement statement, int line_index) {
    return 1;
}

