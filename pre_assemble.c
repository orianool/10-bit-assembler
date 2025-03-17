#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pre_assemble.h"
#include "globals.h"
#include "utils.h"

static Macro_node *macro_list; /* to contain information about macros. */

/* preprocess: Expands macros and deletes comments */
/* FILE *source - input file                       */
/* FILE *output - new file to overwrite            */
/* return - 1 if successful, 0 if failed           */
int pre_assemble(FILE *source, FILE *output) {
    char line[MAX_LINE + 1]; /* char array with enough space for maximum lenght line plus NULL character */
    char* word;
    int i, status;

    macro_list = NULL;
    status = 0;

    word = NULL;

    while((fgets(line, MAX_LINE + 1, source)) != NULL) {
        i = get_word(line, 0, &word); /* gets first word in the line */

        if(i == 0) { /* empty line */
            fprintf(output, "%s", line); /* copies line into output file */
            continue;
        }

        if(is_macro_name(word)) { /* encountered a macro */
            copy_macro(word, output);
            continue;
        }

        if(!strcmp(word, "macro")) { /* word IS a macro */
            if(!make_macro_node(source, line)) { /* failed to create a macro */
                status = 1; /* to indicate that an error occurred */
            }
            continue;
        }
        fprintf(output, "%s", line); /* line is "normal", copies line into output file */
    }

    if(word != NULL) /* if word was not freed by get_word */
        free(word);
    if(macro_list != NULL)
        free_macro_list();

    if (status) {
        return 0;
    }
    return 1;
}

/* make_macro_node: creates a new Macro_node and saves a macro to it from input file */
/* FILE *source - source file                                                        */
/* FILE *output - output file                                                        */
/* return - 1 if successful, 0 if failed                                             */
int make_macro_node(FILE *source, char *declaration) {
    Macro_node *new_macro, *current_node;
    char *macro_name, *tmp_word, line[MAX_LINE];
    int line_num, index;

    macro_name = tmp_word = NULL; /* sets to NULL to avoid errors */

    index = get_word(declaration, 0, &tmp_word); /* to move past "macro" in beginning of the line */

    /* analyze macro declaration */
    index = get_word(declaration, index, &macro_name); /* get the macro name */
    if(get_word(declaration, index, &tmp_word) != 0) { /* if there is another word in the line -> invalid macro name */
        fprintf(stderr, "Invalid macro declaration: additional parameters - %s\n", tmp_word);
        if(macro_name != NULL) /* if word was not freed by get_word */
            free(macro_name);
        return 0;
    }
    if(!is_valid_macro(macro_name)) {
        if(macro_name != NULL) /* if not freed by get_word */
            free(macro_name);
        return 0;
    }


    /* passed checks -> valid macro declaration! */
    if((new_macro = malloc(sizeof(Macro_node))) == NULL) {
        printf("make_macro_node: malloc failed\n");
        exit(0);
    }
    if((new_macro->name = malloc(sizeof(strlen(macro_name) + 1))) == NULL) {
        printf("make_macro_node: malloc failed\n");
        exit(0);
    }
    strcpy(new_macro->name, macro_name);

    /* save lines */
    new_macro->content = malloc(sizeof(char*) * DEFAULT_MACRO_SIZE); /* allocate initial memory for macro content */
    for(line_num = 0 ; /* stop condition is in the loop */ ; line_num++) {
        if((line_num % DEFAULT_MACRO_SIZE) == 0 && (line_num != 0)) { /* if memory allocate is full */
            new_macro->content = realloc(new_macro->content, line_num + DEFAULT_MACRO_SIZE); /* allocate additional memory */
        }

        fgets(line, MAX_LINE + 1, source); /* get next line */
        index = get_word(line, 0, &tmp_word); /* save first word in line in to tmp_word */
        if((tmp_word == NULL) || (strcmp(tmp_word, "endmacro"))) { /* if word IS NOT "endmacro" */
            new_macro->content[line_num] = malloc(MAX_LINE);
            strcpy(new_macro->content[line_num], line);
            continue;
        }

        /* if word IS "endmacro" */
        if ((index = get_word(line, index, &tmp_word)) != 0) { /* check if there are more words after "endmacro" */
            new_macro->content[line_num] = malloc(MAX_LINE);
            strcpy(new_macro->content[line_num], line);
            continue;
        }
        break; /* stop loop if tmp_word is "endmacro" */
    }
    new_macro->line_count = line_num;
    new_macro->next_macro = NULL;

    /*add new macro to macro list */
    if(macro_list == NULL) { /* macro list is empty */
        macro_list = new_macro;
    }
    else {
        current_node = macro_list;
        while(current_node->next_macro != NULL) { /* find the end of macro list */
            current_node = current_node->next_macro;
        }
        current_node->next_macro = new_macro;
    }
    if(macro_name != NULL) /* if not freed by get_word */
        free(macro_name);
    return 1;
}

/* is_valid_macro: checks if a string is a valid macro name */
/* char *name - string to check                             */
/* return - 1 if valid, 0 if NOT valid                      */
int is_valid_macro(char* name) {

    if(is_keyword(name)) {
        fprintf(stderr, "Invalid macro name: is a keyword - %s\n", name);
        return 0;
    }
    if(is_macro_name(name)) {
        fprintf(stderr, "Invalid macro name: %s is already declared\n", name);
        return 0;
    }
    if(name[strlen(name) - 1] == ':') {
        fprintf(stderr, "Invalid macro name: \"%s\" - illegal ':'\n", name);
        return 0;
    }
    return 1;
}


/* is_macro_name: checks if a string is a valid macro name */
/* char *s - string to check                               */
/* return - 0 if valid, 1 if NOT valid                     */
int is_macro_name(char* s) {
    Macro_node *current_node;
    if(macro_list == NULL) {
        return 0;
    }
    current_node = macro_list;
    while(current_node != NULL){
        if (!strcmp(current_node->name, s)) { /* if "s" is a macro name */
            return 1;
        }
        current_node = current_node->next_macro;
    }
    return 0;
}

/* copy_macro: copies a macro's content into a file */
/* char *name - name of the macro to copy           */
/* FILE *output - file to copy into                 */
/* return - void                                    */
void copy_macro(char *name, FILE *output) {
    Macro_node *current_node;
    int line_num;

    /* find the called macro */
    current_node = macro_list;
    while(strcmp(current_node->name, name)) {
        current_node = current_node->next_macro;
    }

    /* copy content into output file */
    for(line_num = 0 ;line_num < current_node->line_count ; line_num++) {
        fprintf(output, "%s", current_node->content[line_num]);
    }
}

/* free_macro_list: frees the macro linked-list */
/* return - void                                */
void free_macro_list() {
    Macro_node *current_node;
    Macro_node *next_node;
    int line_num;

    if (macro_list != NULL) {
        current_node = macro_list;
        next_node = current_node->next_macro;

        while(next_node != NULL) {

            /* free current_node */
            free(current_node->name);
            for(line_num = 0; line_num < current_node->line_count; line_num++) {
                free(current_node->content[line_num]);
            }
            free(current_node->content);

            /* move to next nodes */
            current_node = next_node;
            next_node = next_node->next_macro;
        }
    }
}