
/* TODO - update status after errors - implant in other file too (if necessary) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "first_pass.h"
#include "utils.h"
#include "globals.h"

unsigned int line_num;

static int PASS_STATUS, SECOND_PARAM, FIRST_PARAM_IS_REG; /* flags used in file */
extern unsigned int DC;
extern unsigned int IC;
extern Reg registers[];
extern Command commands[];
extern Instruction instructions[];
extern char line[]; /* will be used by functions in first_pass during file analysis. */
extern char word[]; /* will be used by functions in first_pass during file analysis. */
extern Binary_table_node *binaryTableNode;
extern Label_table_node *label_table;
extern Entry_node *entries_table;
extern Extern_node *extern_table;

/* first_pass: translate from assembly to binary by building 4 table containing the information
   FILE * source - file to translate
   return - 1 if succeeded, 0 if failed */
int first_pass(FILE *source) {
    int i, line_status, line_index, LABEL_FLAG, EXTERN_ENTRY_FLAG;
    char *param;
    char endChr;
    Label_table_node* current_label;
    Binary_table_node* tmp_head;
    func_ptr execute;

    binaryTableNode = NULL;
    entries_table = NULL;
    extern_table = NULL;
    label_table = NULL;
    line_num = 1;
    PASS_STATUS = 0;

    while(get_line(source) != NULL) {
        line_status = 0;
        SECOND_PARAM = 0;
        LABEL_FLAG = 0;
        EXTERN_ENTRY_FLAG = 0;
        tmp_head = NULL;

        if((IC + MEM_DISPLACEMENT) >= MEM_LIMIT) {
            printf("ERROR - program exceeded maximum amount of available memory\n");
            return 0;
        }

        if(is_comment_empty_line(line)) {
            line_num++;
            continue;
        }

        if(strstr(line, ":") != NULL) { /* check for label declaration */
            line_index = get_word_delim(line, 0, ':'); /* get label name */
            if(is_valid_label(word, 1)) {
                if(label_table == NULL) {
                    label_table = make_label(word, IC + MEM_DISPLACEMENT);
                    LABEL_FLAG = 1;
                }
                else {
                    /* advance in label_table until end of list */
                    for(current_label = label_table; current_label->next != NULL; current_label = current_label->next)
                        ;
                    current_label->next = make_label(word, IC + MEM_DISPLACEMENT);
                    LABEL_FLAG = 1;
                }
            }
            line_index = get_word_abs(line, line_index, &endChr); /* get command \ instruction */
        }
        else
            line_index = get_word_abs(line, 0, &endChr); /* get command \ instruction - if no label */

        /* check for illegal characters after command / instruction */
        if(!isalnum(endChr) && isspace(line[line_index])) {
            printf("ERROR - line %i: \"%c\" illegal character after command\\instruction\n",line_num, endChr);
            line_status++;
            line_index++; /* move past illegal character, so it won't get detected by other functions */
        }

        param = line + line_index; /* get parameters */

        if((i = is_command(word)) != NULL_COMMAND) {
            tmp_head = make_binary_table_node((IC++) + MEM_DISPLACEMENT, 0, NULL);
            tmp_head->statement.code = commands[i].code;
            execute = commands[i].function;
            line_status += execute(param, tmp_head);
            tmp_head->statement.code = tmp_head->statement.code << 2; /* moves code to correct position and clears space for ARE */
        }
        else if((i = is_instruction(word)) != NULL_INSTRUCTION) {
            execute = instructions[i].function;
            if(i == 3 || i == 4)
                EXTERN_ENTRY_FLAG = 1;
            if(LABEL_FLAG && EXTERN_ENTRY_FLAG) {
                free_last_label(label_table);
                printf("WARNING - line %i: redundant label declaration\n", line_num);
            }
            if(!EXTERN_ENTRY_FLAG) {
                tmp_head = make_binary_table_node((IC++) + MEM_DISPLACEMENT, 0, NULL);
                if(!LABEL_FLAG) { /* if encountered .data / .string / .struct without label */
                    printf("ERROR - line %i: missing label before .data / .string / .struct\n",line_num);
                    line_status++;
                }
            }
            line_status += execute(param, tmp_head);
        }
        else { /* unknown word */
            printf("ERROR - line %i: \"%s\" unknown command\n",line_num, word);
            line_status++;
        }

        if(line_status) { /* in case encountered errors in line */
            free_Binary_table_list(tmp_head);
            PASS_STATUS++;
        }
        else {
            if(EXTERN_ENTRY_FLAG)
                free_Binary_table_list(tmp_head);
            else
                append_list(&binaryTableNode, &tmp_head);
        }
        line_num++;
    }
    if(PASS_STATUS) {
        return 0; /* first_pass found errors, don't move to second pass */
    }
    return 1; /* no errors found, move to second pass */
}

/* commands */

/* mov: analyzes "mov" command line. saves data in a binary table.
   char *param - parameters given to mov.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int mov(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_one_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: mov: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* cmp: analyzes "cmp" command line. saves data in a binary table.
   char *param - parameters given to cmp.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int cmp(char *param, Binary_table_node *list) {
    int errors;

    errors = type_one_func(param, list);

    return errors;
}

/* add: analyzes "add" command line. saves data in a binary table.
   char *param - parameters given to add.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int add(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_one_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: add: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* sub: analyzes "sub" command line. saves data in a binary table.
   char *param - parameters given to sub.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int sub(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_one_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: sub: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* not: analyzes "not" command line. saves data in a binary table.
   char *param - parameters given to not.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int not(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_two_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: not: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* clr: analyzes "clr" command line. saves data in a binary table.
   char *param - parameters given to clr.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int clr(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_two_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: clr: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* lea: analyzes "lea" command line. saves data in a binary table.
   char *param - parameters given to lea.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int lea(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_SOURCE_ADDRESS_MASK, ILLEGAL_DESTINATION_ADDRESS_MASK, MASK_RES;

    ILLEGAL_DESTINATION_ADDRESS_MASK.code = 3; /* 0000000011 - shift for ARE bits hasn't happened yet */
    ILLEGAL_SOURCE_ADDRESS_MASK.code = 12; /* 0000001100 - shift for ARE bits hasn't happened yet */

    errors = type_one_func(param, list);

    if(errors != 2) {
        MASK_RES.code = ILLEGAL_SOURCE_ADDRESS_MASK.code & list->statement.code;
        if(MASK_RES.code != 1 && MASK_RES.code != 2) { /* if both bits are on or both bits are off */
            printf("ERROR - line %u: lea: illegal source operand\n", line_num);
            return 1;
        }
    }

    if(errors != 3) {
        if(!(ILLEGAL_DESTINATION_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of two bit are on */
            printf("ERROR - line %u: lea: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* inc: analyzes "inc" command line. saves data in a binary table.
   char *param - parameters given to inc.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int inc(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_two_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: inc: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* dec: analyzes "dec" command line. saves data in a binary table.
   char *param - parameters given to dec.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int dec(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_two_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: dec: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* jmp: analyzes "jmp" command line. saves data in a binary table.
   char *param - parameters given to jmp.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int jmp(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_two_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: jmp: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* bne: analyzes "bne" command line. saves data in a binary table.
   char *param - parameters given to bne.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int bne(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_two_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: bne: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* get: analyzes "get" command line. saves data in a binary table.
   char *param - parameters given to get.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int get(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_two_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: get: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* prn: analyzes "prn" command line. saves data in a binary table.
   char *param - parameters given to prn.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int prn(char *param, Binary_table_node *list) {
    int errors;

    errors = type_two_func(param, list);

    return errors;
}

/* jsr: analyzes "jsr" command line. saves data in a binary table.
   char *param - parameters given to jsr.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int jsr(char *param, Binary_table_node *list) {
    int errors;
    Statement ILLEGAL_ADDRESS_MASK;

    ILLEGAL_ADDRESS_MASK.code = 3;

    errors = type_two_func(param, list);

    if(errors != 2) {
        if(!(ILLEGAL_ADDRESS_MASK.code & list->statement.code)) { /* checks if one of the two last bit are on */
            printf("ERROR - line %u: jsr: illegal destination operand\n", line_num);
            errors = 1;
        }
    }

    return errors;
}

/* rts: analyzes "rts" command line
   char *param - parameters given to rts.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int rts(char *param, Binary_table_node *list) {
    int errors;
    errors = type_three_func(param, list);
    return errors;
}

/* hlt: analyzes "cmp" command line.
   char *param - parameters given to hlt.
   *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int hlt(char *param, Binary_table_node *list) {
    int errors;
    errors = type_three_func(param, list);
    return errors;
}

/* instructions */

/* f_data: analyzes .data instruction. saves data in a binary table.
   char *param - parameters given to f_data. must be a list of integer between -128 to 127 divided with ','.
   Binary_table_node *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int f_data(char *param, Binary_table_node *list) {
    char endChr;
    long num;
    int i, errors, sign, data_count;
    Binary_table_node *current;
    Label_table_node *label;

    errors = data_count =  0;
    current = list;

    if(*param == '\0') {
        printf("ERROR - line %u: _data: no arguments\n", line_num);
        return 1;
    }

    i = 0;
    do {
        /* get parameter */
        i = get_word_abs(param, i, &endChr);
        if(i == 0) {
            printf("ERROR - line %u: _data: extraneous comma\n", line_num);
            errors = 1;
        }

        if(!is_valid_int(word))
            errors = 1;
        else {
            sign = get_sign(word);

            if(sign) { /* if negative */
                num = strtol(word + 1, NULL, 10); /* gets number without minus sing */
                current->statement.code = negate_int((int)num);
                data_count++;
            }
            else {
                num = strtol(word, NULL, 10);
                current->statement.code = num;
                data_count++;
            }
        }

        if(endChr != ',' && endChr != '\0') {
            if(endChr != '\0'){
                printf("ERROR - line %u: _data: missing comma\n", line_num);
                errors = 1;
            }
        }
        if(endChr != '\0') {
            current->next = make_binary_table_node((IC++) + MEM_DISPLACEMENT, 0, NULL);
            current = current->next;
        }
    } while(endChr != '\0');

    label = label_table;
    while (label->next != NULL) {
        label = label->next;
    }
    label->data_count = data_count;
    DC += data_count;

    return errors;
}

/* f_string: analyzes .string instruction. saves data in a binary table.
   char *param - parameters given to f_data. must be characters encased in "".
   Binary_table_node *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int f_string(char *param, Binary_table_node *list) {
    char c;
    int i, data_count;
    Binary_table_node *current;
    Label_table_node *label;

    current = list;
    data_count = 0;

    if(*param == '\0') {
        printf("ERROR - line %u: _string: no arguments\n", line_num);
        return 1;
    }

    i = 0;
    if(param[i++] != '"') {
        printf("ERROR - line %u: _string: \"%s\" illegal string\n", line_num, param);
        return 1;
    }
    while((c = *(param + i++)) != '"') {
        if(c == '\0') {
            printf("ERROR - line %u: _string:  missing '\"' at end of string\n", line_num);
            return 1;
        }
        current->statement.code = (unsigned int)c;
        current->next = make_binary_table_node((IC++) + MEM_DISPLACEMENT, 0, NULL);
        current = current->next;
        data_count++;
    }
    current->statement.code = 0;
    data_count++;
    if(get_word_abs(param, i, NULL) != 0) {
        printf("ERROR - line %u: _string: \"%s\" extraneous text after end of string\n", line_num, word);
        return 1;

    }

    label = label_table;
    while (label->next != NULL) {
        label = label->next;
    }
    label->data_count = data_count;
    DC += data_count;

    return 0;
}

/* f_data: analyzes .struct instruction. saves data in a binary table.
   char *param - parameters given to f_data. A list of one integer (between -128 to 127) and
   string (characters encased in ""). integer and string must be separated with a ','.
   Binary_table_node *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int f_struct(char *param, Binary_table_node *list) {
    char endChr, c;
    long num;
    int i, errors, sign, data_count;
    Binary_table_node *current;
    Label_table_node *label;

    errors = data_count = 0;
    current = list;

    if(*param == '\0') {
        printf("ERROR - line %u: _struct: no arguments\n", line_num);
        return 1;
    }

    /* get first parameter */
    i = get_word_abs(param, 0, &endChr);
    if(!is_valid_int(word))
        errors = 1;
    else {
        sign = get_sign(word);

        if(sign) { /* if negative */
            num = strtol(word + 1, NULL, 10); /* gets number without minus sing */
            current->statement.code = negate_int((int)num);
            data_count++;
        }
        else {
            num = strtol(word, NULL, 10);
            current->statement.code = num;
            data_count++;
        }
    }

    if(endChr != ',' && endChr != '\0') {
        if(endChr != '\0'){
            printf("ERROR - line %u: _struct: missing comma\n", line_num);
            errors = 1;
        }
    }
    else if(endChr == '\0') {
        if(endChr != '\0'){
            printf("ERROR - line %u: _struct: missing string\n", line_num);
            return 1;
        }
    }

    current->next = make_binary_table_node((IC++) + MEM_DISPLACEMENT, 0, NULL);
    current = current->next;
    get_word_delim(param, i, '\n');
    i = 0;
    if(word[i++] != '"') {
        printf("ERROR - line %u: _struct: \"%s\" illegal string\n", line_num, param);
        return 1;
    }
    while((c = *(word + i++)) != '"') {
        if(c == '\0') {
            printf("ERROR - line %u: _struct:  missing '\"' at end of string\n", line_num);
            return 1;
        }
        current->statement.code = (unsigned int)c;
        current->next = make_binary_table_node((IC++) + MEM_DISPLACEMENT, 0, NULL);
        current = current->next;
        data_count++;
    }
    current->statement.code = 0;
    data_count++;
    if(get_word_delim(word, i, '\n') != 0) {
        printf("ERROR - line %u: _struct: \"%s\" extraneous text after end of string\n", line_num, word);
        return 1;
    }

    label = label_table;
    while (label->next != NULL) {
        label = label->next;
    }
    label->data_count = data_count;
    DC += data_count;

    return errors;
}

/* f_entry: analyzes .entry instruction. saves data in an entries table.
   char *param - name of entry.
   Binary_table_node *list - not used in function, can be set to NULL.
   return - 0 if succeeded, non 0 if failed */
int f_entry(char *param, Binary_table_node *list) {
    int i, errors;
    char endChr;
    Entry_node *entry, *current;

    errors = 0;

    if(*param == '\0') {
        printf("ERROR - line %u: _entry: no arguments\n", line_num);
        return 1;
    }

    i = get_word_abs(param, 0, &endChr);
    if(endChr != '\0') {
        printf("ERROR - line %u: _entry: \"%s\" extraneous text after label\n", line_num, param + i);
        errors = 1;
    }

    if(is_entry(word)) { /* entry is already defined elsewhere */
        printf("WARNING - line %i: entry is already declared\n", line_num);
        return 0;
    }
    if(is_extern(word)) {
        printf("ERROR - line %u: _entry: \"%s\" entry is already defined as extern\n", line_num, word);
        return 1;
    }

    if(!is_valid_label(word, 0)) {
        /* error is printed by is_valid_label */
        return 1;
    }
    entry = make_entry(word, 0);
    if(entries_table == NULL)
        entries_table = entry;
    else {
        current = entries_table;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = entry;
    }
    return errors;
}

/* f_extern: analyzes .extern instruction. saves data in an entries table.
   char *param - name of extern.
   Binary_table_node *list - not used in function, can be set to NULL.
   return - 0 if succeeded, non 0 if failed */
int f_extern(char *param, Binary_table_node *list) {
    int i, errors;
    char endChr;
    Extern_node *new_extern, *current;

    errors = 0;

    if(*param == '\0') {
        printf("ERROR - line %u: _entry: no arguments\n", line_num);
        return 1;
    }

    i = get_word_abs(param, 0, &endChr);
    if(endChr != '\0') {
        printf("ERROR - line %u: _entry: \"%s\" extraneous text after label\n", line_num, param + i);
        errors = 1;
    }

    if(is_extern(word)) { /* extern is already defined elsewhere */
        printf("WARNING - line %i: extern is already declared\n", line_num);
        return 0; /* entry is already defined elsewhere */
    }
    if(is_entry(word)) {
        printf("ERROR - line %u: _entry: \"%s\" entry is already defined as extern\n", line_num, word);
        return 1;
    }

    if(!is_valid_label(word, 0)) {
        /* error is printed by is_valid_label */
        return 1;
    }
    new_extern = make_extern(word);
    if(extern_table == NULL)
        extern_table = new_extern;
    else {
        current = extern_table;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_extern;
    }
    return errors;
}

/* analyze_param: analyzes a parameter according to addressing type
   char *param - parameter to analyze
   Binary_table_node *list - list to save data to
   return - 0 if succeeded, non 0 if failed */
int analyze_param(char* param, Binary_table_node *list) {
    Address address;
    char *endPtr;
    int sign, i, struct_field;
    long num;
    Statement statement;

    address = get_address(param);

    switch (address.val) {
        case 0:
            get_word_abs(param, 1, NULL); /* saves the param without '#' in to word */
            if(is_valid_int(word)) {
                sign = get_sign(word);
                if(sign) { /* if negative */
                    num = strtol(word + 1, &endPtr, 10); /* gets number without minus sing */
                    add_code(negate_int((int)num), list);
                }
                else {
                    num = strtol(word, &endPtr, 10);
                    add_code((int)num, list);
                }
            }
            else /* not a valid integer */
                return 0;
            break;
        case 1:
            if(!is_valid_label(param, 0))
                return 0;
            add_code(0, list); /* adds an empty word */
            break;
        case 2:
            i = get_word_delim(param, 0, '.');
            if(!is_valid_label(word, 0))
                return 0;
            i = get_word_abs(param, i, NULL);
            if(*word == '\0') {
                printf("ERROR - line %u: missing struct field\n", line_num);
                return 0;
            }
            struct_field = (int)strtol(word, &endPtr, 10);
            if((struct_field != 1 && struct_field != 2) || *endPtr != '\0') {
                printf("ERROR - line %u: \"%s\" is invalid struct field\n", line_num, param);
                return 0;
            }
            add_code(0, list); /*adds an empty word for address of struct */
            add_code(struct_field, list); /* adds word with struct field number */
            break;
        case 3: /* get_address already checked if valid register name */
            for(i = 0; strcmp(registers[i].name, param); i++)
                ;
            if(!SECOND_PARAM) { /* register is first parameter */
                add_code(registers[i].reg_num << 4, list);
                FIRST_PARAM_IS_REG = 1;
            }
            else { /* register second parameter */
                statement.code = registers[i].reg_num;
                statement.code = statement.code << 2;
                if(FIRST_PARAM_IS_REG) {
                    /* add second register number to first register word */
                    list->next->statement.code += statement.code;
                }
                else
                    add_code(registers[i].reg_num, list);
            }

            break;
        default:
            printf("ERROR - line %u: \"%s\" invalid parameter\n", line_num, word);
            return 0;
    }
    list->statement.code = list->statement.code << 2;
    list->statement.code += address.val;
    return 1;
}

/* type_one_func: analyzes a command that gets two operands
   char *param - the operands separated with ','
   Binary_table_node *list - list to save the date in to
   return - 0 if succeeded, non 0 if failed */
int type_one_func(char *param, Binary_table_node *list) {
    int i, errors, command_index;
    char endChr;

    command_index = list->statement.code;
    errors = 0;
    FIRST_PARAM_IS_REG = 0;

    /* get and analyze first parameter */
    i = get_word_abs(param, 0, &endChr);
    if(i == 0) {
        printf("ERROR - line %u: %s: missing source operand\n", line_num, commands[command_index].name);
        errors = 2;
        return errors;
    }
    if(!analyze_param(word, list))
        errors = 1;

    if(endChr != ',' && endChr != '\0') {
        printf("ERROR - line %u: %s: missing comma\n", line_num, commands[command_index].name);
        errors = 1;
    }

    /* get and analyze second parameter */
    SECOND_PARAM = 1; /* to indicate entering the analysis of second parameter */
    i = get_word_abs(param, i, &endChr);
    if(i == 0) {
        printf("ERROR - line %u: %s: missing destination operand\n", line_num, commands[command_index].name);
        errors = 3;
        return errors;
    }
    if(endChr != '\0') {
        printf("ERROR - line %u: %s: extraneous text after parameters\n", line_num, commands[command_index].name);
        errors = 1;
    }
    if(!analyze_param(word, list))
        errors = 1;

    return errors;
}

/* type_two_func: analyzes a command that gets one operand.
   char *param - the operand.
   Binary_table_node *list - list to save the date in to
   return - 0 if succeeded, non 0 if failed */
int type_two_func(char *param, Binary_table_node *list) {
    int i, errors, command_index;
    char endChr;

    command_index = list->statement.code;
    errors = 0;

    list->statement.code = list->statement.code << 2; /* no source operand */

    /* get and analyze first parameter */
    i = get_word_abs(param, 0, &endChr);
    if(i == 0) {
        printf("ERROR - line %u: %s: missing argument\n", line_num, commands[command_index].name);
        errors = 2;
        return errors;
    }
    if(!analyze_param(word, list))
        errors = 1;

    if(endChr != '\0') {
        printf("ERROR - line %u: %s: extraneous text after parameters\n", line_num, commands[command_index].name);
        errors = 1;
    }

    return errors;
}

/* type_three_func: analyzes a command that gets no operands
   char *param - the operand.
   Binary_table_node *list - list to save the date in to
   return - 0 if succeeded, non 0 if failed */
int type_three_func(char *param, Binary_table_node *list) {
    int errors, command_index;
    char endChr;

    command_index = list->statement.code;
    errors = 0;

    /* get and analyze first parameter */
    get_word_abs(param, 0, &endChr);

    if(strlen(word) != 0) {
        printf("ERROR - line %u: %s: extraneous text after command\n", line_num , commands[command_index].name);
        errors = 1;
    }

    list->statement.code = list->statement.code << 4; /* because there are no calls for analyze_parameter */
    return errors;
}