
#include <stdio.h>
#include <string.h>
#include "makefiles.h"
#include "globals.h"
#include "utils.h"

extern Binary_table_node *binaryTableNode;
extern Label_table_node *label_table;
extern Entry_node *entries_table;
extern Extern_call_node *extern_call_list;
extern unsigned int DC;
extern unsigned int ILC;
static char base32[] = {'!','@','#','$','%','^','&','*','<','>','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v'};

/* makefiles: creates files in machine code (base 32). no errors will be detected in these stage
   char *f_name - name of the files created
   return - 1 */
int makefiles(char* f_name) {
    if(binaryTableNode != NULL)
        make_object_file(f_name);
    if(entries_table != NULL)
        make_entries_file(f_name);
    if(extern_call_list != NULL)
        make_external_file(f_name);
    return 1;
}

/* fprint_binary_to_32: prints a number in base 32 to a given file
    no need for loop or a complicated algorithm because num can equal to 1023 maximum
    and therefore num in base 32 will be 2 digits at most.
    FILE *fp - file to print to.
    unsigned int num - number to converts.
    return - void */
void fprint_binary_to_32(FILE *fp, unsigned int num) {
    unsigned int res;

    if(!num) { /* if num is 0 */
        fprintf(fp, "%c", base32[num]);
        return;
    }

    fprintf(fp, "%c", base32[(res = num / 32)]);
    num %= 32;
    fprintf(fp, "%c", base32[num]);
    return;
}

/* make_object_file: creates an .ob and converts and translates the binary table in to the .ob file
   char *f_name - name of file created
   return - 0 if no file was created, 1 if file was created */
int make_object_file(char* f_name) {
    FILE *out;
    Binary_table_node *binary;

    char file_name[MAX_FILENAME_LEN];
    char *ob_extension = ".ob";

    if(binaryTableNode == NULL)
        return 0;

    /* build file name including extension */
    strcpy(file_name, f_name);
    strcat(file_name, ob_extension);

    binary = binaryTableNode;
    out = fopen(file_name, "w");

    /* prints header */
    fprintf(out, "   ");
    fprint_binary_to_32(out, ILC);
    fprintf(out," ");
    fprint_binary_to_32(out, DC);
    fprintf(out,"\n");

    /* prints body */
    for (binary = binaryTableNode; binary != NULL; binary = binary->next) {
        fprint_binary_to_32(out, binary->mem_address);
        fprintf(out, "\t");
        fprint_binary_to_32(out, binary->statement.code);
        fprintf(out, "\n");
    }
    return 1;
}

/* make_entries_file: creates an .ent and converts and translates the entries table in to the .ent file
   char *f_name - name of file created
   return - 0 if no file was created, 1 if file was created */
int make_entries_file(char* f_name) {
    Entry_node *entryNode;
    FILE *out;

    char file_name[MAX_FILENAME_LEN];
    char *ob_extension = ".ent";

    if(entries_table == NULL)
        return 0;

    /* build file name including extension */
    strcpy(file_name, f_name);
    strcat(file_name, ob_extension);

    out = fopen(file_name, "w");

    for(entryNode = entries_table; entryNode != NULL; entryNode = entryNode->next) {
        fprintf(out, "%-30s ",entryNode->name); /* prints 30 places to ensure nice output and because maximum label name length is 30 */
        fprint_binary_to_32(out, entryNode->mem_address);
        fprintf(out, "\n");
    }
    free_entry_list(entries_table);
    return 1;
}

/* make_external_file: creates an .ext and converts and translates the entries table in to the .ext file
   char *f_name - name of file created
   return - 0 if no file was created, 1 if file was created */
int make_external_file(char* f_name) {
    Extern_call_node *externCallNode;
    FILE *out;
    char file_name[MAX_FILENAME_LEN];
    char *ob_extension = ".ext";
    
    if(extern_call_list == NULL)
        return 0;

    /* build file name including extension */
    strcpy(file_name, f_name);
    strcat(file_name, ob_extension);

    out = fopen(file_name, "w");

    for(externCallNode = extern_call_list; externCallNode != NULL; externCallNode = externCallNode->next) {
        fprintf(out, "%-30s ",externCallNode->name); /* prints 30 places to ensure nice output and because maximum label name length is 30 */
        fprint_binary_to_32(out, externCallNode->mem_address);
        fprintf(out, "\n");
    }
    free_extern_call_list(extern_call_list);
    return 1;
}