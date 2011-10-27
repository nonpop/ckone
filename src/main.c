#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "args.h"
#include "log.h"
#include "ckone.h"


extern bool parse_args (int argc, char** argv);
extern bool ckone_init (s_ckone* kone);
extern bool ckone_load (s_ckone* kone, FILE* input);
extern int ckone_run (s_ckone* kone);
extern void ext_init_devices ();


int main (int argc, char** argv) {
    if (!parse_args (argc, argv))
        return EXIT_FAILURE;

    FILE* program_file = NULL;
    if (!strcmp (args.program, "-")) {
        program_file = stdin;
        ILOG ("Reading the program from standard input.\n", 0);
    } else {
        program_file = fopen (args.program, "r");
        ILOG ("Reading the program from %s\n", args.program);
    }

    if (!program_file) {
        ELOG ("Cannot open %s for reading\n", args.program);
        return EXIT_FAILURE;
    }

    s_ckone kone;
    if (!ckone_init (&kone))
        return EXIT_FAILURE;

    if (!ckone_load (&kone, program_file))
        return EXIT_FAILURE;

    ext_init_devices ();
    return ckone_run (&kone);
}

