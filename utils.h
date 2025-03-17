#ifndef MAMAN14_UTILS_H
#define MAMAN14_UTILS_H

#include "globals.h"

/* functions declaration */
int get_word(char*, int, char**);
int get_word_delim(char *str, int i, char delim);
int get_word_abs(char *str, int i, char *endChr);
int get_sign(char *num);
char* get_line(FILE *source);

void add_code(unsigned int num, Binary_table_node *list);
int negate_int(int num);
void replace_code(unsigned int pos, Statement statement);

int is_comment_empty_line(char *str);
int is_keyword(char* s);
int is_command(char *s);
int is_instruction(char *s);
int is_valid_int(char *num);

Binary_table_node* make_binary_table_node(unsigned int mem, unsigned int statement, Binary_table_node *next);
void free_Binary_table_list(Binary_table_node *head);
void append_list(Binary_table_node **destination, Binary_table_node **source);

Label_table_node* make_label(char *s, unsigned int i);
void free_label_list(Label_table_node *list);
void free_label_node(Label_table_node* node);
void free_last_label(Label_table_node *list);
int is_label(char *str);
int is_valid_label(char *s, int check_list);

Extern_node* make_extern(char* str);
void add_extern_call_node(char *name, unsigned int mem_address);
void free_extern_call_node(Extern_call_node *callNode);
void free_extern_call_list(Extern_call_node *list);
int is_extern(char *str);

Entry_node* make_entry(char* str, unsigned int mem);
void free_entry_list(Entry_node *list);
void free_extern_node(Extern_node *node);
void free_extern_list(Extern_node *list);
int is_entry(char *str);

#endif