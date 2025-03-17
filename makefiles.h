
#ifndef MAMAN14_MAKEFILES_H
#define MAMAN14_MAKEFILES_H

/* functions declarations */
int makefiles(char* f_name);
void fprint_binary_to_32(FILE *fp, unsigned int num);
int make_object_file(char* f_name);
int make_entries_file(char* f_name);
int make_external_file(char* f_name);

#endif
