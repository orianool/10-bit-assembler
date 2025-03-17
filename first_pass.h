
#ifndef MAMAN14_FIRST_PASS_H
#define MAMAN14_FIRST_PASS_H

#include "globals.h"

/* functions declarations */
int first_pass(FILE *);
Address get_address(char *op);
int analyze_param(char* param, Binary_table_node *current);
int type_one_func(char *param, Binary_table_node *list);
int type_two_func(char *param, Binary_table_node *list);
int type_three_func(char *param, Binary_table_node *list);

/* commands */
int mov(char *param, Binary_table_node *list);
int cmp(char *param, Binary_table_node *list);
int add(char *param, Binary_table_node *list);
int sub(char *param, Binary_table_node *list);
int not(char *param, Binary_table_node *list);
int clr(char *param, Binary_table_node *list);
int lea(char *param, Binary_table_node *list);
int inc(char *param, Binary_table_node *list);
int dec(char *param, Binary_table_node *list);
int jmp(char *param, Binary_table_node *list);
int bne(char *param, Binary_table_node *list);
int get(char *param, Binary_table_node *list);
int prn(char *param, Binary_table_node *list);
int jsr(char *param, Binary_table_node *list);
int rts(char *param, Binary_table_node *list);
int hlt(char *param, Binary_table_node *list);

/* instructions */
int f_data(char *param, Binary_table_node *list);
int f_string(char *param, Binary_table_node *list);
int f_struct(char *param, Binary_table_node *list);
int f_entry(char *param, Binary_table_node *list);
int f_extern(char *param, Binary_table_node *list);

#endif
