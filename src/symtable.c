#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "log.h"


typedef struct s_symtable {
    char* name;
    int value;
    char* value_str;
    struct s_symtable* next;
} s_symtable;


static s_symtable* symtable = NULL;


static s_symtable* allocate_node (char* name, char* value) {
    s_symtable* new = malloc (sizeof (s_symtable));
    if (!new)
        return NULL;

    new->name = malloc ((strlen (name) + 1)*sizeof (char));
    if (!new->name) {
        free (new);
        return NULL;
    }

    new->value_str = malloc ((strlen (value) + 1)*sizeof (char));
    if (!new->value_str) {
        free (new->name);
        free (new);
        return NULL;
    }

    return new;
}


bool symtable_insert (char* name, char* value) {
    s_symtable* new = allocate_node (name, value);
    if (!new) {
        ELOG ("Failed to allocate memory for a symbol table node\n", 0);
        return false;
    }

    strcpy (new->name, name);
    strcpy (new->value_str, value);
    sscanf (value, "%d", &new->value);

    new->next = symtable;
    symtable = new;

    return true;
}


s_symtable* find_symbol (char* name) {
    for (s_symtable* s = symtable; s; s = s->next)
        if (!strcmp (s->name, name))
            return s;
    return NULL;
}


bool symtable_lookup (char* name, int* value) {
    s_symtable* s = find_symbol (name);
    if (!s)
        return false;

    *value = s->value;
    return true;
}


bool symtable_lookup_str (char* name, char** value) {
    s_symtable* s = find_symbol (name);
    if (!s)
        return false;

    *value = s->value_str;
    return true;
}


void symtable_dump () {
    printf ("Symbol table:\n");
    for (s_symtable* s = symtable; s; s = s->next)
        printf ("%s = %s\n", s->name, s->value_str);
}

