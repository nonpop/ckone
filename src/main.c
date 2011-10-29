/**
 * @file main.c
 *
 * The program entry point. Parses the command line arguments and
 * calls functions to initialize and start the emulator.
 * Used external functions: ckone_init(), ckone_load(), ckone_run(),
 * ckone_free(), ext_init_devices() and ext_close_devices().
 *
 * The main page of the documentation is also included here. Doxygen
 * will render a nice html page from it; see the README file.
 */


/**
 * @mainpage
 *
 * @section intro Introduction
 *
 * Ckone is a program which aims to emulate the execution of a TTK-91 processor.
 * The instruction set is described at 
 * http://www.cs.helsinki.fi/group/titokone/v1.200/kaskyt_en.txt .
 * Ckone implements the complete instruction set, and it supports the @c DEF 
 * directive, which means that if the symbol table of the program file contains
 * an @c stdin or @c stdout entry, then reading and writing from these devices will 
 * be directed to the given files. The SVC routines have also been implemented.
 *
 * <em>Titokone 1.203</em> (http://www.cs.helsinki.fi/group/titokone/) has been used 
 * as a reference when implementing @e ckone. Two bugs were found in @e Titokone, 
 * which can be replicated in @e ckone by using the @c --emulate-bugs -flag. The bugs 
 * are:
 *  -# The SVC @c READ routine takes two arguments instead of one, and ignores the 
 *     second-pushed argument. The following code (see samples/bug1.[k91|b91]) 
 *     illustrates this:
@verbatim
    A DC 0
    B DC 1337       ; A random value to show that the call doesn't 
    PUSH SP, =A     ; seem to care at all about the second argument.
    PUSH SP, =B
    SVC SP, =READ
    SVC SP, =HALT
@endverbatim
 *     Run this in <em>Titokone 1.203</em> and enter, say, 42 as input. The result is 
 *     that the memory location @e A will contain 42, and @e B remains 1337. Also, 
 *     the value of @c SP indicates that the SVC really took two arguments. Running 
 *     this in @e ckone without the @c --emulate-bugs flag enabled will result in 
 *     @e B getting the value 42 and the address of @e A remaining on the stack after
 *     the call.
 *  -# The SVC @c DATE routine reports the month as one too small. The following 
 *     code (see samples/bug2.[k91|b91]) illustrates this:
@verbatim
    DAY DC 0
    MONTH DC 0
    YEAR DC 0
    PUSH SP, =YEAR
    PUSH SP, =MONTH
    PUSH SP, =DAY
    SVC SP, =DATE
    SVC SP, =HALT
@endverbatim
 *     Running this in @e ckone without the @c --emulate-bugs flag enabled will report
 *     the correct month.
 *
 *
 * @section compilation Compilation
 * 
 * The compilation process is described in the README file.
 *
 *
 * @section running Running
 *
 * The syntax for running is <tt>ckone [OPTIONS...] program.b91</tt>. The program 
 * file should be a "binary" file assembled by, for example, @e Titokone. If a dash 
 * (-) is used, the program is read from the standard input. The available options 
 * can be listed using <tt>ckone --help</tt>.
 * 
 * The amount of logging messages can be adjusted by using the @c --verbose flag. If 
 * it is not used at all, only the most important messages are shown (warnings, 
 * errors, prompts and dumps). If the flag is used once, information messages are 
 * also shown, for example initialization messages and the currently executing 
 * instruction. Using the flag twice causes all messages to be included. This means 
 * lots of output which is only useful for debugging. More details about what the 
 * other options mean can be found in the following section.
 *
 *
 * @section structure The structure of the program
 * 
 * The @c ckone executable consists of two parts: the emulator and the interface. 
 * The emulator is built from the files alu.c, cpu.c, ext.c, instr.c, and mmu.c.
 * The interface is built from ckone.c, symtable.c, and main.c. The files 
 * args.c and log.c are linked in the emulator library since they are also used 
 * by the test module.
 *
 * The most important data structure is ::s_ckone. It holds the values of all
 * registers and contains a pointer to the emulator memory. Any operation which
 * does not interact with the outside world can be completed using only
 * information in this structure. The operations using an external device
 * will get the device data from the ::devices table. The @c DATE and @c TIME SVC
 * routines use functions in the C library to get the necessary information.
 *
 * The symbol table is a linked list of ::s_symtable nodes. It is used to 
 * figure out the @c STDIN and @c STDOUT device files defined in the program file.
 * The contents of the table can also be included in memory dumps to facilitate
 * following the execution of the emulator.
 *
 * One more data structure is the ::s_arguments struct, which is used to store
 * the command-line-modifiable options. First, the structure is initialized
 * with default values, and then the command line parser modifies it according
 * to the arguments given by the user. The rest of the program will then adjust
 * its behavior according to the values in the structure.
 * 
 * @subsection initialization Initialization
 *
 * When the program starts, it first parses the command line using Argp 
 * (http://www.gnu.org/s/hello/manual/libc/Argp.html), which will initialize 
 * the ::args variable. After that, the ::s_ckone structure is initialized 
 * using ckone_init(). This allocates memory for the emulator, the amount of 
 * which can be modified using the @c --mem-size command line argument. It also 
 * sets the MMU base and limit registers (adjustable by @c --mmu-base and 
 * @c mmu-limit respectively) and, if the @c --zero flag was used, fills the memory
 * and other registers with zeroes. The @c PC and @c SR registers and the halted -flag are cleared in any case 
 * to reset the CPU.
 *
 * After this the program is loaded into memory using ckone_load(). The program 
 * is loaded so that the first word is put to the memory address indicated by 
 * the MMU base register, which may not be zero. The @c FP and @c SP registers are 
 * set to point to the end of code and data segments respectively (relative to 
 * the MMU base register), the way @e Titokone does.
 *
 * Then the external devices (@c KBD, @c CRT, @c STDIN and @c STDOUT) are initialized.
 * @c KBD is connected with the standard input and @c CRT with the standard output. 
 * If the @c --stdin option was used, then the given file is opened and assigned to 
 * the @c STDIN device. If the option was not used, but the program's symbol table 
 * contains the symbol @c stdin, then the value of that symbol is used. <em>This only 
 * works correctly if the stdin symbol with the filename comes after the @e stdin 
 * symbol with the device number in the input file.</em> However, @e Titokone cannot 
 * handle this situation either, so it is probably safe to assume that the file name 
 * symbols are always last in the table. If the filename symbol happens to come 
 * before the device number symbol, you have to use the @c --stdin option to use the 
 * correct file.
 *
 * If the @c --stdin option is not used and the program file does not define a 
 * @c stdin file, the file @c stdin in the current working directory is used. 
 * Everything said for the @c STDIN device also applies to the @c STDOUT device 
 * (@c --stdout and @c stdout file).
 *
 * The program tries to open both of these files regardless of whether they are 
 * going to be used or not. This means two things. First, if the file for @c STDIN 
 * does not exist, a warning will be printed. Second, the file for @c STDOUT will 
 * be created. These are minor annoyances, but they should be fixed some day.
 *
 * @subsection emulation Emulation
 *
 * Next the emulator is started (ckone_run()). If the @c --step flag was used, the 
 * emulator will pause and dump the memory and registers before every instruction.
 * Otherwise, the emulator will run until the CPU is halted (using SVC @c HALT) or 
 * some error occurs, stopping only to get user input when the program reads from 
 * the @c KBD device. After the emulator has finished, the contents of memory and 
 * the registers will always be shown. The memory is printed with several memory 
 * locations on one row. The length of a row (the number of columns) can be adjusted 
 * by using the @c --columns option and the number base can be changed with the 
 * @c --base option. If the @c --show-symtable flag was set, each dump will also 
 * contain the contents of the symbol table.
 *
 * Each instruction is executed in the following steps:
 *  -# The MMU is told to fetch the next instruction from the memory address 
 *     given by the @c PC register, and after that the @c PC is incremented by one.
 *  -# The second operand of the instruction is calculated. First the contents 
 *     of the index register (if some other than @c R0) is added to the constant 
 *     part of the instruction. Then, if the addressing mode is not @c IMMEDIATE, 
 *     a new value will be retrieved from that location. If the addressing
 *     mode is @c INDIRECT, yet another value is retrieved from the location given 
 *     by the previous value. This value is then stored in the @c TR register.
 *  -# A suitable function is called based on the instrument's operation code. 
 *     The function then performs the operation and the cycle repeats.
 *
 * All memory accessing is done through the MMU functions in mmu.c. These first 
 * convert the address given in @c MAR (a logical address), which is relative to 
 * the MMU base register, into a physical address, which is relative to the 
 * beginning of the memory. If this address is within the limits of the MMU base
 * and limit registers, the memory operation is performed. Otherwise, the ::SR_M 
 * bit of @c SR is set. If the operation was a read operation, the result is stored 
 * in the @c MBR register. If the operation was a write operation, the value in 
 * @c MBR is stored into memory at the calculated physical location.
 * 
 * The ALU operations are in alu.c. They all read the @c ALU_IN1 and @c ALU_IN2 
 * registers (except the @c NOT operation only reads @c ALU_IN1), and store their 
 * result in @c ALU_OUT. The ::SR_O and ::SR_Z bits of the @c SR register are set 
 * if needed.
 * 
 * The operations @c IN, @c OUT and @c SVC are defined in ext.c, although the SVC 
 * call is prepared in cpu.c by pushing the @c PC and @c FP registers onto the stack 
 * and, when the call exits, they are popped back and a proper amount is subtracted 
 * from @c SP to remove the arguments from the stack. The operations which perform 
 * I/O will set the ::SR_M bit of @c SR if an I/O error occurs. The SVC @c HALT 
 * routine is the only way to stop a program without signaling an error.
 *
 * The rest of the operations (@c STORE, @c LOAD, @c COMP, and the jump and stack 
 * operations) are all defined in cpu.c.
 * 
 *
 * @section log Logging
 *
 * The file log.c contains a very simple logger and some helper macros are in 
 * log.h. The logger is just a wrapper for @c printf, printing the given message 
 * only if the message is important enough given the current verbosity level of 
 * the program.
 *
 *
 * @section tests Testing
 *
 * The other executable, @c ckone_tests was built from the emulator part of @e ckone 
 * and the files in the @c test directory. It contains unit-tests for most of the 
 * operations which do not access the external world. The tests produce lots of 
 * messages, but they can be ignored if the last line says that all tests were 
 * passed. This should be the case.
 *
 * The @c samples subdirectory contains example programs, most of which are from 
 * http://www.cs.helsinki.fi/group/nodes/kurssit/tito/esimerkit/ and 
 * http://www.cs.helsinki.fi/group/nodes/kurssit/tito/esim_vanhat/ . The sources 
 * were compiled using <em>Titokone 1.203</em>. The results when running with 
 * <em>Titokone 1.203</em> and @e ckone with @c --emulate-bugs should be identical, 
 * with the exception of the registers and memory locations which were not written 
 * by the program, and the final @c PC value (@e ckone does not make it -1 when 
 * halting).
 *
 */

#include <argp.h>
#include "common.h"
#include "ext.h"
#include "args.h"
#include "config.h"


extern bool ckone_init (s_ckone* kone);
extern bool ckone_load (s_ckone* kone, FILE* input);
extern int ckone_run (s_ckone* kone);
extern void ckone_free (s_ckone* kone);


/// @cond skip
const char* argp_program_version = "ckone " VERSION;

static char doc[] = 
"ckone -- a ttk-91 emulator\v"
"If the program file is -, the program is read from the standard input\n"
"The stdin and stdout options override settings defined in the program file.\n";

static char args_doc[] = "PROGRAM_FILE";


// a little trick to get an integer constant converted to a string constant
#define STR_(i) #i
#define STR(i) STR_(i) 


static struct argp_option options[] = {
    { "stdin",          'i',    "INFILE",   0, 
        "Use INFILE as the STDIN device", 0 },
    
    { "stdout",         'o',    "OUTFILE",  0, 
        "Use OUTFILE as the STDOUT device", 0 },

    { "mem-size",       'm',    "SIZE",     0, 
        "Use SIZE words of memory (default: " STR(DEFAULT_MEMORY_SIZE) ")", 0 },

    { "mmu-base",       300,    "BASE",     0, 
        "Set mmu_base to BASE (default: 0)", 0 },
    
    { "mmu-limit",      301,    "LIMIT",    0, 
        "Set mmu_limit to LIMIT (default: mem_size)", 0 },

    { "zero",           'z',    0,          0, 
        "Fill memory and registers with zero before starting", 0 },

    { "columns",        'c',    "COLS",     0, 
        "Use COLS columns in memory dumps (default: " STR(DEFAULT_MEMDUMP_COLUMNS) ")", 0 },

// A little exercise in conditional compilation
#if DEFAULT_MEMDUMP_BASE == 10
    { "base",           'b',    0,          0, 
        "Use hexadecimal numbers in memory dumps", 0 },
#elif DEFAULT_MEMDUMP_BASE == 16
    { "base",           'b',    0,          0,
       "Use decimal numbers in memory dumps", 0 },
#else
#error Invalid value for DEFAULT_MEMDUMP_BASE
#endif

    { "step",           's',    0,          0, 
        "Pause execution after each instruction", 0 },

    { "verbose",        'v',    0,          0, 
        "Be verbose (use twice to be very verbose)", 0 },

    { "emulate-bugs",   400,    0,          0, 
        "Emulate bugs found in TitoKone 1.203", 0 },

    { "show-symtable",  'y',    0,          0, 
        "Include the symbol table in dumps", 0 },
    
    { 0, 0, 0, 0, 0, 0 }    // end of table
};

/// @endcond


/**
 * @internal
 * The option parser. Used by Argp.
 *
 * @return See the Argp documentation.
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
        case 'z':
            arguments->zero = true;
            break;
        case 'c':
            arguments->mem_cols = atoi(arg);
            break;
        case 'b':
            arguments->mem_swap_base = true;
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

/// @cond skip
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };
/// @endcond


/**
 * @internal
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


/**
 * @internal
 * Parse and validate the command line arguments.
 *
 * @return True if the parsing and validation succeeded.
 */
static bool 
parse_args (
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
    args.zero = false;
    args.mem_cols = DEFAULT_MEMDUMP_COLUMNS;
    args.mem_swap_base = false;
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
    DLOG ("zero = %s\n", bool_to_yesno (args.zero));
    DLOG ("mem_cols = %d\n", args.mem_cols);
    DLOG ("mem_swap_base = %s\n", bool_to_yesno (args.mem_swap_base));
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
    if (args.mmu_base >= args.mem_size) {
        ELOG ("mmu_base must be less than mem_size\n", 0);
        return false;
    }
    if (args.mmu_limit < 0)
        args.mmu_limit = args.mem_size - args.mmu_base;

    if (args.mmu_base + args.mmu_limit > args.mem_size) {
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
 *
 * @return EXIT_SUCCESS if the program terminated normally, or
 *         EXIT_FAILURE if something went wrong.
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

    if (program_file != stdin)
        fclose (program_file);

    // Init the external devices.
    ext_init_devices ();

    // Run the emulator.
    int retval = ckone_run (&kone);

    // Clean up.
    ext_close_devices ();
    ckone_free (&kone);

    return retval;
}

