/**
 * @file main.c
 *
 * The program entry point. Parses the command line arguments and
 * calls functions to initialize and start the emulator.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include "args.h"
#include "log.h"
#include "ckone.h"
#include "config.h"
#include "ext.h"


/// @cond private
const char* argp_program_version = VERSION;

static char doc[] = 
"ckone -- a ttk-91 emulator\v"
"If the program file is -, the program is read from the standard input\n"
"The stdin and stdout options override settings defined in the program file.\n";

static char args_doc[] = "PROGRAM_FILE";


// a little trick to get an integer constant converted to a string constant
#define STR_(i) #i
#define STR(i) STR_(i) 


static struct argp_option options[] = {
    { "stdin",          'i',    "INFILE",   0, "Use INFILE as the STDIN device", 0 },
    { "stdout",         'o',    "OUTFILE",  0, "Use OUTFILE as the STDOUT device", 0 },
    { "mem-size",       'm',    "SIZE",     0, "Use SIZE words of memory (default: " STR(DEFAULT_MEMORY_SIZE) ")", 0 },
    { "mmu-base",       300,    "BASE",     0, "Set mmu_base to BASE (default: 0)", 0 },
    { "mmu-limit",      301,    "LIMIT",    0, "Set mmu_limit to LIMIT (default: mem_size)", 0 },
    { "clean",          302,    0,          0, "Fill memory and registers with zero before starting", 0 },
    { "columns",        'c',    "COLS",     0, "Use COLS columns in the memory dump (default: " STR(DEFAULT_MEMDUMP_COLUMNS) ")", 0 },
    { "step",           's',    0,          0, "Pause execution after each instruction", 0 },
    { "verbose",        'v',    0,          0, "Be verbose (use twice to be very verbose)", 0 },
    { "emulate-bugs",   400,    0,          0, "Emulate bugs found in TitoKone 1.203", 0 },
    { "show-symtable",  'y',    0,          0, "Include the symbol table in dumps", 0 },
    { 0, 0, 0, 0, 0, 0 }
};


/**
 * The option parser. Used by Argp.
 */
static error_t
parse_opt (
        int key,                    ///< Argument type.
        char* arg,                  ///< Argument value.
        struct argp_state *state    ///< Parser state.
        ) 
{
    s_arguments* arguments = state->input;

    switch (key) {
        case 'i':
            arguments->stdin_file = arg;
            break;
        case 'o':
            arguments->stdout_file = arg;
            break;
        case 's':
            arguments->step = true;
            break;
        case 'v':
            arguments->verbosity++;
            break;
        case 'm':
            arguments->mem_size = atoi(arg);
            break;
        case 300:
            arguments->mmu_base = atoi(arg);
            break;
        case 301:
            arguments->mmu_limit = atoi(arg);
            break;
        case 302:
            arguments->clean = true;
            break;
        case 'c':
            arguments->mem_cols = atoi(arg);
            break;
        case 400:
            arguments->emulate_bugs = true;
            break;
        case 'y':
            arguments->include_symtable = true;
            break;

        case ARGP_KEY_ARG:
            if (state->arg_num >= 1)
                argp_usage (state);

            arguments->program = arg;
            break;

        case ARGP_KEY_END:
            if (state->arg_num < 1)
                argp_usage (state);
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };


/**
 * Convert a boolean to a "yes" or "no" string.
 *
 * @return A "yes" or "no" string.
 */
static char* 
bool_to_yesno (
        bool value      ///< The value to convert.
        ) 
{
    return value? "yes" : "no";
}

/// @endcond


/**
 * Parse and validate the command line arguments.
 *
 * @return True if the parsing and validation succeeded.
 */
bool parse_args (
        int argc,       ///< The size of the argv array.
        char** argv     ///< The actual arguments.
        ) 
{
    // Initialize the options structure.
    args.stdin_file = NULL;
    args.stdout_file = NULL;
    args.mem_size = DEFAULT_MEMORY_SIZE;
    args.mmu_base = 0;
    args.mmu_limit = -1;
    args.clean = false;
    args.mem_cols = DEFAULT_MEMDUMP_COLUMNS;
    args.step = false;
    args.verbosity = 0;
    args.emulate_bugs = false;
    args.program = NULL;
    args.include_symtable = false;

    // Parse
    argp_parse (&argp, argc, argv, 0, 0, &args);

    DLOG ("stdin_file = %s\n", args.stdin_file);
    DLOG ("stdout_file = %s\n", args.stdout_file);
    DLOG ("mem_size = %d\n", args.mem_size);
    DLOG ("mmu_base = %d\n", args.mmu_base);
    DLOG ("mmu_limit = %d\n", args.mmu_limit);
    DLOG ("clean = %s\n", bool_to_yesno (args.clean));
    DLOG ("mem_cols = %d\n", args.mem_cols);
    DLOG ("step = %s\n", bool_to_yesno (args.step));
    DLOG ("verbosity = %d\n", args.verbosity);
    DLOG ("emulate_bugs = %s\n", bool_to_yesno (args.emulate_bugs));
    DLOG ("program = %s\n", args.program);
    DLOG ("include_symtable = %s\n", bool_to_yesno (args.include_symtable));


    // Validate the arguments.
    
    if (args.mem_size <= 0) {
        ELOG ("mem_size must be positive\n", 0);
        return false;
    }
    if (args.mmu_base < 0) {
        ELOG ("mmu_base must be non-negative\n", 0);
        return false;
    }
    if ((size_t)args.mmu_base >= args.mem_size) {
        ELOG ("mmu_base must be less than mem_size\n", 0);
        return false;
    }
    if (args.mmu_limit < 0)
        args.mmu_limit = args.mem_size - args.mmu_base;

    if ((size_t)(args.mmu_base + args.mmu_limit) > args.mem_size) {
        ELOG ("mmu_base + mmu_limit must be at most mem_size\n", 0);
        return false;
    }

    if (args.mem_cols <= 0) {
        ELOG ("mem_cols must be positive\n", 0);
        return false;
    }

    if (args.verbosity > 2) {
        args.verbosity = 2;
        ILOG ("Verbosity limited to 2\n", 0);
    }

    return true;
}


/**
 * The program entry point.
 */
int 
main (
        int argc,       ///< The number of command line arguments.
        char** argv     ///< The command line arguments.
        ) 
{
    // Parse the command line arguments.
    if (!parse_args (argc, argv))
        return EXIT_FAILURE;

    // Initialize the emulator.
    s_ckone kone;
    if (!ckone_init (&kone))
        return EXIT_FAILURE;

    // Load the program.
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

    if (!ckone_load (&kone, program_file))
        return EXIT_FAILURE;

    // Init the external devices.
    ext_init_devices ();

    // Run the emulator.
    int retval = ckone_run (&kone);

    // Clean up.
    ext_close_devices ();
    ckone_free (&kone);

    return retval;
}

