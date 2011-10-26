#include <argp.h>
#include "args.h"
#include "version.h"
#include "ckone.h"


const char* argp_program_version = VERSION;


static char doc[] = 
"ckone -- a TitoKone emulator\v"
"If the program file is -, the program is read from the standard input\n"
"The stdin and stdout options override settings defined in the program file.\n";


static char args_doc[] = "PROGRAM_FILE";


static struct argp_option options[] = {
    { "stdin",          'i',    "INFILE",   0,  "Use INFILE as the STDIN device", 0 },
    { "stdout",         'o',    "OUTFILE",  0,  "Use OUTFILE as the STDOUT device", 0 },
    { "mem-size",       'm',    "SIZE",     0,  "Use SIZE words of memory (default: " DEFAULT_MEM_SIZE_STR ")", 0 },
    { "mmu-base",       300,    "BASE",     0,  "Set mmu_base to BASE (default: 0)", 0 },
    { "mmu-limit",      301,    "LIMIT",    0,  "Set mmu_limit to LIMIT (default: mem_size)", 0 },
    { "step",           's',    0,          0,  "The execution pauses after each instruction", 0 },
    { "verbose",        'v',    0,          0,  "Be verbose (warning: produces lots of output)", 0 },
    { "emulate-bugs",   400,    0,          0,  "Emulate a bug found in TitoKone 1.203", 0 },
    { 0, 0, 0, 0, 0, 0 }
};


static error_t
parse_opt (int key, char* arg, struct argp_state *state) {
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
            arguments->verbose = true;
            break;
        case 300:
            arguments->emulate_bugs = true;
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


s_arguments args;

bool parse_args (int argc, char** argv) {
    args.stdin_file = "stdin";
    args.stdout_file = "stdout";
    args.mem_size = DEFAULT_MEM_SIZE;
    args.mmu_base = 0;
    args.mmu_limit = -1;
    args.step = false;
    args.verbose = false;
    args.emulate_bugs = false;
    args.program = NULL;

    argp_parse (&argp, argc, argv, 0, 0, &args);

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

    return true;
}

