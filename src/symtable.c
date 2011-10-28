/**
 * @file symtable.c
 *
 * Code to create and search a symbol table.
 */

#include "common.h"


/**
 * @internal
 * One node in the linked list making up the symbol table.
 */
typedef struct s_symtable {
    char* name;                 ///< The name (key) of the symbol.
    int value;                  ///< The integer value of the symbol. This
                                ///< is undefined for symbols stdin and stdout.
    char* value_str;            ///< The value of the symbol as a string.
    struct s_symtable* next;    ///< A pointer to the next node in the list.
} s_symtable;


/**
 * @internal
 * The first node of the symbol table.
 */
static s_symtable* symtable = NULL;


/**
 * @internal
 * Allocate a new symbol table node.
 *
 * @return NULL if the allocation failed.
 */
static s_symtable* 
allocate_node (
        char* name,     ///< The name of the symbol (used to allocate enough memory).
        char* value     ///< The string value of the symbol (used to allocate enough memory).
        ) 
{
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


/**
 * Clear the symbol table. Frees all nodes and sets
 * the first node to NULL.
 */
void 
symtable_clear (
        void
        ) 
{
    DLOG ("Freeing symbol table...\n", 0);
    for (s_symtable* s = symtable; s; ) {
        s_symtable* next = s->next;
        free (s);
        s = next;
    }
    symtable = NULL;
}


/**
 * Insert a new symbol to the table.
 *
 * @return True if successful.
 */
bool 
symtable_insert (
        char* name,     ///< The name of the symbol.
        char* value     ///< The string value of the symbol.
        ) 
{
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


/**
 * @internal
 * Find a node in the table by its name.
 *
 * @return NULL if no symbol with this name exists in the table.
 */
static s_symtable* 
find_symbol (
        char* name          ///< The symbol name.
        ) 
{
    for (s_symtable* s = symtable; s; s = s->next)
        if (!strcmp (s->name, name))
            return s;
    return NULL;
}


/**
 * Lookup a symbol in the table.
 *
 * @return False if no symbol with this name exists in the table.
 */
bool 
symtable_lookup (
        char* name,         ///< The symbol name.
        int* value          ///< A pointer to a variable where the value should be stored.
        ) 
{
    s_symtable* s = find_symbol (name);
    if (!s)
        return false;

    *value = s->value;
    return true;
}


/**
 * Lookup a symbol in the table.
 *
 * @note The value written by this function will become invalid after
 * symtable_clear() has been called.
 *
 * @return False if no symbol with this name exists in the table.
 */
bool 
symtable_lookup_str (
        char* name,         ///< The symbol name.
        char** value        ///< A pointer to a variable where the value should be stored.
        ) 
{
    s_symtable* s = find_symbol (name);
    if (!s)
        return false;

    *value = s->value_str;
    return true;
}


/**
 * Print the symbol table.
 */
void 
symtable_dump (
        void
        ) 
{
    printf ("Symbol table:\n");
    for (s_symtable* s = symtable; s; s = s->next)
        printf ("%s = %s\n", s->name, s->value_str);
}

