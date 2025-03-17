
#ifndef MAMAN14_SECOND_PASS_H
#define MAMAN14_SECOND_PASS_H

#include "globals.h"

/* functions declarations */
int second_pass(FILE *);
Statement get_binary(unsigned int pos);
int get_type(char *str);
int f_type_one(Statement statement, int i);
int f_type_two(Statement statement, int line_index);
int f_type_three(Statement statement, int line_index);

#endif
