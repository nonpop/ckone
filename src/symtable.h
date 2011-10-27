#ifndef SYMTABLE_H
#define SYMTABLE_H


extern bool symtable_insert (char* name, char* value);
extern bool symtable_lookup (char* name, int* value);
extern bool symtable_lookup_str (char* name, char** value);
extern void symtable_dump ();
extern void symtable_clear ();


#endif

