#ifndef MAMAN14_PRE_ASSEMBLR_H
#define MAMAN14_PRE_ASSEMBLR_H

#define DEFAULT_MACRO_SIZE 4

/* Implementation of linked list of macros */
typedef struct macro_node{
	char *name;
	char** content;
	int line_count;
	struct macro_node *next_macro;
} Macro_node;

/* functions declaration */
int pre_assemble(FILE *, FILE *);
int make_macro_node(FILE *source, char *declaration);
int is_valid_macro(char* name);
int is_macro_name(char* s);
void free_macro_list();
void copy_macro(char *name, FILE *output);
#endif