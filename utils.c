#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "first_pass.h"
#include "utils.h"

#define INT_IN_RANGE(num) ((sign && num <= MAX_INT) || num < MAX_INT)

char line[MAX_LINE]; /* will be used by functions in first_pass during file analysis. */
char word[MAX_LINE - 1]; /* will be used by functions in first_pass during file analysis. */

extern unsigned long DC, IC;
extern Command commands[];
extern Instruction instructions[];
extern Reg registers[];
extern unsigned int line_num;
extern Entry_node  *entries_table;
extern Extern_node *extern_table;
extern Binary_table_node *binaryTableNode;
extern Extern_call_node *extern_call_list;
extern Label_table_node *label_table;

/* get_word: finds the first word in a line from specified index and saves it to a given character array */
/* assumes start is smaller than length of str. if word is not found, sets the variable "word" to null                */
/* char *str - line to search                                                                           */
/* int start - index to start search from                                                               */
/* char **word - save into                                                                              */
/* returns index of the next character after the found word                                              */
int get_word(char* str, int start, char** wrd) {
	char c;
	int end, i;
	
	/* finds the beginning of a word */
	for(; isspace(c = *(str + start)); start++)
		;

	if(c == '\n' || c == '\0') { /* beyond start str is empty or only white characters */
		if(*wrd != NULL) /* first use of get_word */
			free(*wrd);
		*wrd = NULL;
		return 0;
	} 
			

	/* finds the end of the word */
	for(end = start; !isspace(c = *(str + end)) && c != '\0'; end++)
		; 

	if(*wrd == NULL) { /* first use of get_word */
		*wrd = malloc((end - start) + 1); /* no need for sizeof */
	}
	else{ /* NOT first use of get_word */
		*wrd = realloc(*wrd, (end - start) + 1);

		/* free(*word);
		*word = malloc(end - start + 1); no need for sizeof */
	}
	

	/* copies first word into word */
	for(i = 0; start < end; start++, i++) {
		*(*wrd + i) =  *(str + start);
	}
	*(*wrd + i) = '\0';

	return end;
}

/* is_keyword: checks if a string is a keyword    */
/* char *s - string to check                      */
/* return - 1 if IS a keyword, 0 if NOT a keyword */
int is_keyword(char* s) {
	int i;
	char *tmp;

	/* is register */
	for(i = 0; (tmp = registers[i].name) != NULL; i++)
		if (!strcmp(tmp, s)) { /* if "s" is a register name */
			return 1;
	}

	/* is command */
	for(i = 0; (tmp = commands[i].name) != NULL; i++)
		if (!strcmp(tmp, s)) { /* if "s" is a register name */
			return 1;
	}

	return 0;
}

/* get_line: saves a line from file in to an external 'char line[]' variable. is limited to MAX_LINE characters.
   FILE *source - source file.
   return - pointer to 'char line[]'. */
char* get_line(FILE *source) {
    int c;

    if(fgets(line, MAX_LINE + 1, source) == NULL) /* end of file */
        return NULL;
    if((*(line + (strlen(line) - 1)))!= '\n') { /* if \n is not the last char in line than there are additional chars left after the 81st char */
        printf("Error: line %u - Line is to long\n", line_num);
        for(; (c = fgetc(source)) != '\n' && c != '\0'; ) /* get to end of line */
            ;
    }
    return line;
}

/* get_word_delim: saves part of a string using a delimiter in to external 'char word[]' variable.
   char *str - source string.
   int i - index to start from.
   char delim - deliminator character.
   return - int indicating position after deliminator. 0 if end of string */
int get_word_delim(char *str, int i, char delim) {
    char c;
    int count;

    /* finds the beginning of a word */
    for(; isspace(c = *(str + i)); i++)
        ;

    if(c == '\n' || c == '\0') /* beyond start str is empty or only white characters */
        return 0;

    /*saves the word found into the static variable word */
    for(count = 0; (c = *(str + i)) != '\0' && c != delim; count++, i++) {
        *(word + count) =  *(str + i);
    }
    *(word + count) = '\0';

    return ++i; /* the index of the end of the word found in str */
}

/* get_word_abs: saves a word from a string in to external 'char word[]' variable.
   white spaces and ',' are deliminators.
   char *str - source string.
   int i - index to start from.
   return - int indicating position of next word. 0 if end of string */
int get_word_abs(char *str, int i, char *endChr) {
    char c;
    int count;

    /* finds the beginning of a word */
    for(; isspace(c = *(str + i)); i++)
        ;

    if(c == '\n' || c == '\0' || c == ',') {  /* beyond start str is empty or only white characters */
        word[0] = '\0'; /* "empties" word */
        if(endChr != NULL)
            *endChr = '\0';
        return 0;
    }

    /*saves the word found into the variable word */
    for(count = 0; (c = *(str + i)) != '\0' && !isspace(c) && c != ','; count++, i++) {
        *(word + count) =  *(str + i);
    }
    *(word + count) = '\0';

    /* moves to start of new word */
    for(; (c = *(str + i)) != '\0' && isspace(c); i++)
        ;

    if(endChr != NULL)
        *endChr = c;

    if(c == ',') /* moves past the ',' */
        return ++i;
    return i; /* the index of the end of the word found in str */
}

/* is_command: checks if s is a command.
   char *s - string to check.
   return - NULL_COMMAND if not, int representing index in commands array/ */
int is_command(char *s) {
    int i;
    char *tmp_name;

    i = 0;
    tmp_name = commands[i].name;
    while(tmp_name != NULL) {
        if(!strcmp(tmp_name, s)) {
            return i;
        }
        tmp_name = commands[++i].name;
    }
    return NULL_COMMAND; /* 16 is the index of the NULL command */
}

/* is_instruction: checks if s is an instruction.
   char *s - string to check.
   return - NULL_INSTRUCTION if not, int representing index in instructions array/ */
int is_instruction(char *s) {
    int i;
    char *tmp_name;

    i = 0;
    tmp_name = instructions[i].name;
    while(tmp_name != NULL) {
        if(!strcmp(tmp_name, s)) {
            return i;
        }
        tmp_name = instructions[++i].name;
    }
    return NULL_INSTRUCTION; /* 5 is the index of the NULL instruction */
}

/* is_label: checks is str is in the label table.
   char *str - name of label (including ':').
   return - 1 if IS label, 0 if NOT label. */
int is_label(char *str) {
    Label_table_node* current_label;
    int len;
    char *tmp;

    if((len = strlen(str)) <= 1) /* str is empty or only 1 character -> can't be a label */
        return 0;
    if(*(str + (len - 1)) != ':') /* str does NOT end with ':' -> str is NOT a label */
        return 0;

    if((tmp = malloc(len)) == NULL) {
        printf("ERROR: malloc failed\n");
        exit(0);
    }
    strncpy(tmp, str, len - 1); /* copies str to tmp without ':' */
    *(tmp + (len - 1)) = '\0';

    for(current_label = label_table; current_label != NULL; current_label = current_label->next) {
        if(!strcmp(tmp, current_label->name)) {
            return 1;
        }
    }
    return 0;
}

/* is_valid_label: checks if s is a valid label. can check if exists in label list or not.
   int check_list - to indicate if necessary to check if s is in label_list (0 to skip, non 0 to check)
   return - 0 if NOT valid, 1 if IS valid */
int is_valid_label(char *s, int check_list) {
    Label_table_node* current_label;
    unsigned i, len;

    len = strlen(s);
    if(len > 30){
        /*need to find a way to print line of error */
        printf("ERROR - line %u: label name is to long\n", line_num);
        return 0;
    }
    for(i = 0; i < len - 1; i++) {
        if(!isalnum(*(s + i))) { /* s contains a non-alphanumeric character -> s is NOT a label */
            printf("ERROR - line %u: \"%s\" label name contains illegal characters\n", line_num, s);
            return 0;
        }
    }

    if(is_keyword(s)) {
        printf("ERROR - line %u: label name \"%s\" is a keyword\n", line_num, s);
        return 0;
    }

    if(check_list) {
        if(label_table != NULL) { /* if label list is empty */
            /* go over label list and check if s is already in list */
            for(current_label = label_table; current_label != NULL; current_label = current_label->next) {
                if(!strcmp(current_label->name, s)) {
                    /*need to find a way to print line of error */
                    printf("ERROR - line %u: \"%s\" label name already used\n", line_num, s);
                    return 0;
                }
            }
        }
    }
    return 1;
}

/* make_binary_table_node: creates a new Binary_table_node.
   unsigned int mem - memory address.
   unsigned int statement - data to store in Statement.
   Binary_table_node *next - next Binary_table_node.
   return - pointer to the new node */
Binary_table_node* make_binary_table_node(unsigned int mem, unsigned int statement, Binary_table_node *next) {
    Binary_table_node *new_node = malloc(sizeof(Binary_table_node));
    new_node->mem_address = mem;
    new_node->statement.code = statement;
    new_node->next = next;
    return new_node;
}

/* free_Binary_table_list: frees entire list of Binary_table_node.
   Binary_table_node *head - head of list to free.
   return - void. */
void free_Binary_table_list(Binary_table_node *head) {
    Binary_table_node *current, *prev;

    if(head == NULL)
        return;
    current = head;

    do {
        prev = current;
        current = current->next;
        free(prev); /* no need for special free function */
    } while(current != NULL);
}

/* make_binary_table_node: creates a new Label_table_node.
   char *s - name of label.
   unsigned int i - position in instruction image.
   return - pointer to the new node */
Label_table_node* make_label(char *s, unsigned int i) {
    Label_table_node* new_node;

    if((new_node = malloc(sizeof(Label_table_node))) == NULL) { /* memory for node */
        printf("make_label: malloc failed\n");
        exit(0);
    }

    if((new_node->name = malloc(strlen(s))) == NULL) { /* memory for name of label */
        printf("make_label: malloc failed\n");
        exit(0);
    }

    strcpy(new_node->name, s); /* set the name */
    new_node->address = i; /* set address */
    new_node->data_count = 0;
    new_node->next = NULL;
    return new_node;
}

/* free_label_node: frees a Label_table_node.
   Label_table_node *node - Label_table_node to free.
   return - void. */
void free_label_node(Label_table_node* node) {
    free(node->name);
    free(node);
}

/* free_label_list: frees entire list of Label_table_node.
   Label_table_node *list - head of list to free.
   return - void. */
void free_label_list(Label_table_node *list) {
    Label_table_node *current, *prev;

    if(list == NULL)
        return;
    current = list;

    do {
        prev = current;
        current = current->next;
        free_label_node(prev); /* no need for special free function */
    } while(current != NULL);
}

/* get_address: gets the addressing type of an operand.
   char *op - the operand.
   return - Address initialized with the addressing type. */
Address get_address(char *op) {
    Address res;
    char *tmp;
    int i;
    extern Address address_type[];

    if(op[0] == '#') {
        res.val = 0;
        res.words = address_type[res.val].words;
        return  res;
    }
    /* is register */
    for(i = 0; (tmp = registers[i].name) != NULL; i++)
        if (!strcmp(tmp, op)) { /* if "op" is a register name */
            res.val = 3;
            res.words = address_type[res.val].words;
            return res;
        }

    /* is struct */
    if(strchr(op,'.') != NULL) {
        if(op[0] != '.') { /* to make sure '.' is not the first character in op */
            res.val = 2;
            res.words = address_type[res.val].words;
            return res;
        }
    }
    res.val = 1;
    res.words = address_type[res.val].words;
    return  res;
}

/* get_sign: gets the sing of and integer in the shape of a string.
   char *num - number to check.
   return - 1 if found a leading minus sign. IMPORTANT: for ALL other cases reruns 0. use with care! */
int get_sign(char *num) {
    char c;
    int i;

    for (i = 0; !isspace(c = *(num + i)) && c != '\0' ; i++) {
        if(c == '-')
            return 1;
    }
    return 0;
}

/* is_valid_int: checks if num (in shape of a string) is a valid integer.
   char *num - string to check.
   return - 1 if IS valid, 0 if NOT valid */
int is_valid_int(char *num) {
    long int res;
    int sign;
    char *endPtr;

    sign = get_sign(num);
    if(sign)
        res = strtol(num + 1, &endPtr, 10);
    else
        res = strtol(num, &endPtr, 10);

    if(*endPtr == '\0') {
        if(INT_IN_RANGE(res)) {
            return 1;
        }
        printf("ERROR - line %u: \"%s\" unsupported integer\n", line_num, word);
        return 0;
    }
    printf("ERROR - line %u: \"%s\" not an integer\n", line_num, word);
    return 0;
}


/* add_code: saves code in next available location in list
   unsigned int code - code to save. IMPORTANT: will be saved in to bit field of size 10.
   return - void. */
void add_code(unsigned int code, Binary_table_node *list) {
    Binary_table_node *current;

    current = list;
    while(current->next != NULL) { /* find next empty spase */
        current = current->next;
    }
    current->next = make_binary_table_node((IC++) + MEM_DISPLACEMENT, code, NULL); /* creates new node */

    current->next->statement.code = current->next->statement.code << 2; /* moves to right position and clears ARE bits */
}

/* negate_int: turn num into negative using 2's compliment.
   int num - number to negate.
   return - negated num */
int negate_int(int num) {
    num = ~num;
    num++;

    return num;
}

/* append_list: append destination source list to destination list.
   Binary_table_node **destination - list to add to.
   Binary_table_node **source - list to be added.
   return - void */
void append_list(Binary_table_node **destination, Binary_table_node **source) {
    Binary_table_node *tmp;

    if(*destination == NULL) {
        *destination = *source;
        return;
    }

    for(tmp = *destination; tmp->next != NULL; tmp = tmp->next)
        ;

    tmp->next = *source;
}

/* make_entry: create a new Entry_node.
   char* str - name of entry.
   unsigned int mem - address in memory.
   return - pointer to the new node */
Entry_node* make_entry(char* str, unsigned int mem) {
    Entry_node *new_entry;

    if((new_entry = malloc(sizeof(Entry_node))) == NULL) { /* memory for node */
        printf("make_label: malloc failed\n");
        exit(0);
    }

    if((new_entry->name = malloc(strlen(str))) == NULL) { /* memory for name of label */
        printf("make_label: malloc failed\n");
        exit(0);
    }

    strcpy(new_entry->name, str); /* set the name */
    new_entry->mem_address = mem; /* set address */
    new_entry->next = NULL;
    return new_entry;
}

/* free_entry_node: frees an Entry_node
   Entry_node *Node - node to free.
   return - void */
void free_entry_node(Entry_node *Node) {
    free(Node->name);
    free(Node);
    return;
}

/* free_entry_list: frees an entire entries list.
   Entry_node *list - head of list to be freed.
   return - void */
void free_entry_list(Entry_node *list) {
    Entry_node *current, *prev;

    if(list == NULL)
        return;
    current = list;

    do {
        prev = current;
        current = current->next;
        free_entry_node(prev);
    } while(current != NULL);
}

/* make_extern: create a new Extern_node.
   char* str - name of extern.
   return - pointer to the new node */
Extern_node* make_extern(char* str) {
    Extern_node *new_extern;

    if((new_extern = malloc(sizeof(Entry_node))) == NULL) { /* memory for node */
        printf("make_label: malloc failed\n");
        exit(0);
    }

    if((new_extern->name = malloc(strlen(str))) == NULL) { /* memory for name of label */
        printf("make_label: malloc failed\n");
        return NULL;
    }

    strcpy(new_extern->name, str); /* set the name */
    new_extern->next = NULL;
    return new_extern;
}

/* free_extern_node: frees an Extern_node
   Extern_node *Node - node to free.
   return - void */
void free_extern_node(Extern_node *Node) {
    free(Node->name);
    free(Node);
    return;
}

/* free_extern_list: frees an entire entries list.
   Extern_node *list - head of list to be freed.
   return - void */
void free_extern_list(Extern_node *list) {
    Extern_node *current, *prev;

    if(list == NULL)
        return;
    current = list;

    do {
        prev = current;
        current = current->next;
        free_extern_node(prev);
    } while(current != NULL);
}

/* is_extern: check if str is in extern list
   char *str - name of extern.
   return - 0 if NOT, 1 if IS. */
int is_extern(char *str) {
    Extern_node *current;

    if(entries_table == NULL)
        return 0;

    current = extern_table;
    while (current != NULL) {
        if(!strcmp(str, current->name))
            return 1;
        current = current->next;
    }
    return 0;
}

/* is_entry: check if str is in entries list
   char *str - name of entry.
   return - 0 if NOT, 1 if IS. */
int is_entry(char *str) {
    Entry_node *current;

    if(entries_table == NULL)
        return 0;

    current = entries_table;
    while (current != NULL) {
        if(!strcmp(str, current->name))
            return 1;
        current = current->next;
    }
    return 0;
}

/* free_last_label: frees the last label node in label list.
   Label_table_node *list - list of which the last node will be freed.
   return - void */
void free_last_label(Label_table_node *list) {
    Label_table_node *prev, *current;

    if(list == NULL)
        return;

    prev = current = list;

    if(current->next == NULL) {
        free_label_node(current);
        label_table = NULL;
        return;
    }

    current = current->next;
    while (current->next != NULL) {
        current = current->next;
        prev = prev->next;
    }
    free_label_node(current);
    prev->next = NULL;
    return;
}

/* is_comment_empty_line: checks if str is a comment of if is empty
  char *str - string to check.
  return 1 if IS comment \ empty, 0 if NOT. */
int is_comment_empty_line(char *str) {
    int i;

    i = get_word_abs(str, 0, NULL);
    if (!i)  /* if i is 0 -> line is empty */
        return 1;
    if(word[0] == ';') /* first character in str is ';' -> line is a comment */
        return 1;
    return 0;
}

/* replace_code: replaces the code in statement.
   unsigned int pos - code to save.
   Statement statement - statement to save code in to.
   return - void */
void replace_code(unsigned int pos, Statement statement) {
    int i;
    Binary_table_node *current;

    pos -= MEM_DISPLACEMENT;
    current = binaryTableNode;

    for(i = 0; i < pos; i++) /* get to binary node in pos */
        current = current->next;

    current->statement.code = statement.code;

    return;
}

/* add_extern_call_node: add a new add_extern_call_node to extern_call_list.
   char *name - name of extern
   unsigned int mem_address - memory address.
   return - void */
void add_extern_call_node(char *name, unsigned int mem_address) {
    Extern_call_node *callNode, *tmp;

    if((callNode = malloc(sizeof(Extern_call_node))) == NULL) { /* memory for node */
        printf("make_label: malloc failed\n");
        exit(0);
    }

    if((callNode->name = malloc(strlen(name))) == NULL) { /* memory for name of label */
        printf("make_label: malloc failed\n");
        exit(0);
    }

    strcpy(callNode->name, name); /* set the name */
    callNode->mem_address = mem_address; /* set address */
    callNode->next = NULL;

    if(extern_call_list == NULL) {
        extern_call_list = callNode;
        return;
    }

    for(tmp = extern_call_list; tmp->next != NULL; tmp = tmp->next) /* find next available place */
        ;

    tmp->next = callNode;
    return;
}

/* free_extern_node: frees an Extern_call_node
   Extern_call_node *callNode - node to free.
   return - void */
void free_extern_call_node(Extern_call_node *callNode) {
    free(callNode->name);
    free(callNode);
    return;
}

/* free_extern_call_list: frees an entire Extern_call_node list.
   Extern_call_node *list - head of list to be freed.
   return - void */
void free_extern_call_list(Extern_call_node *list) {
    Extern_call_node *current, *prev;

    if(list == NULL)
        return;
    current = list;

    do {
        prev = current;
        current = current->next;
        free_extern_call_node(prev);
    } while(current != NULL);
}
