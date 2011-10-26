#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "args.h"
#include "log.h"
#include "ckone.h"


extern void parse_args (int argc, char** argv);
extern bool ckone_init (s_ckone* kone);
extern bool ckone_load (s_ckone* kone, FILE* input);
extern int ckone_run (s_ckone* kone);
extern void ext_init_devices (FILE* stdin_file, FILE* stdout_file);


int main (int argc, char** argv) {
    if (!parse_args (argc, argv))
        return EXIT_FAILURE;

    FILE* stdin_file = fopen (args.stdin_file, "r");
    if (!stdin_file)
        WLOG ("Cannot open %s for reading; trying to read from STDIN will not work\n",
                args.stdin_file);
    
    FILE *stdout_file = fopen (args.stdout_file, "w");
    if (!stdout_file)
        WLOG ("Cannot open %s for writing; trying to write to STDOUT will not work\n",
                args.stdout_file);
    
    FILE* program_file = NULL;
    if (!strcmp (args.program, "-"))
        program_file = stdin;
    else
        program_file = fopen (args.program, "r");

    if (!program_file) {
        ELOG ("Cannot open %s for reading\n", args.program);
        return EXIT_FAILURE;
    }

    s_ckone kone;
    if (!ckone_init (&kone))
        return EXIT_FAILURE;

    if (!ckone_load (&kone, program_file))
        return EXIT_FAILURE;

    ext_init_devices (stdin_file, stdout_file);
    return ckone_run (&kone);
}

