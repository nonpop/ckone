/**
 * @file ckone.c
 *
 * Contains code to initialize and run the emulator.
 * External functions used: symtable_insert() and symtable_clear() to
 * create/destroy the symbol table and symtable_dump() to print the
 * contents of it. To get a textual representation of an instruction,
 * instr_string() is used. The most important, however, is the use of
 * cpu_step() to advance the emulator.
 */

#include "common.h"
#include "instr.h"
#include "cpu.h"
#include "symtable.h"
#include "args.h"
#include "config.h"


/**
 * Initializes the ckone. Allocates memory and resets the CPU.
 * If the zero flag (see ::args) is set, it will also zero all 
 * memory and registers. See also ckone_free().
 *
 * @return True if successful, false otherwise.
 */
bool 
ckone_init (
        s_ckone* kone       ///< The state structure.
        ) 
{
    DLOG ("Initializing the ckone structure...\n", 0);
    if (args.zero) {
        ILOG ("Zeroing state structure...\n", 0);
        memset (kone, 0, sizeof(s_ckone));
    }

    DLOG ("Allocating emulator memory...\n", 0);
    kone->mem = malloc (args.mem_size*sizeof(int32_t));
    if (!kone->mem) {
        ELOG ("Could not allocate %d bytes of memory\n", args.mem_size*sizeof(int32_t));
        return false;
    }
    DLOG ("Allocated %d bytes of memory\n", args.mem_size*sizeof(int32_t));

    if (args.zero) {
        ILOG ("Zeroing emulator memory...\n", 0);
        memset (kone->mem, 0, args.mem_size*sizeof(int32_t));
    }

    kone->mem_size = args.mem_size;
    kone->mmu_base = args.mmu_base;
    kone->mmu_limit = args.mmu_limit;

    kone->pc = 0;
    kone->sr = 0;
    kone->halted = false;

    return true;
}


/**
 * @internal
 * Read a line from the given file. Also update the line number variable.
 *
 * @return The line read, or NULL if there was an error.
 */
static char* 
read_line (
        FILE* input,        ///< The file to read from.
        int* linenum        ///< A pointer to the line number counter.
        ) 
{
    static char buf[1024];
    if (!fgets (buf, sizeof(buf), input)) {
        ELOG ("Failed to read from program file\n", 0);
        return NULL;
    }
    (*linenum)++;
    DLOG ("Line %d = %s", *linenum, buf);
    return buf;
}


/**
 * Load a program into memory. Also sets FP and SP to match the
 * end of the code segment and the data segment respectively.
 * See also ckone_free(). The first word of the program is written
 * to the location pointed by MMU_BASE.
 *
 * @return True if successful, false otherwise.
 */
bool 
ckone_load (
        s_ckone* kone,      ///< The state structure.
        FILE* input         ///< The input file.
        ) 
{
    DLOG ("Reading the program file...\n", 0);

    int linenum = 0;
    char* line;

    /// @cond skip
    // little macros to make things easier to read
#define READ_CHECK() if (!(line = read_line (input, &linenum))) return false
#define EXPECTED(what) { \
    ELOG ("Expected " what " at line %d but got %s\n", linenum, line); \
    return false; \
}
    /// @endcond

    // identifier
    READ_CHECK ();
    if (strcmp (line, "___b91___\n"))
        EXPECTED ("___b91___");

    // code segment
    READ_CHECK ();
    if (strcmp (line, "___code___\n"))
        EXPECTED ("___code___");

    int32_t start, end;
    READ_CHECK ();
    if (sscanf (line, "%d %d", &start, &end) != 2)
        EXPECTED ("two integers");
    
    DLOG ("Code segment: %d - %d\n", start, end);
    kone->r[FP] = end;
    ILOG ("Frame pointer set to 0x%x\n", end);

    for (int32_t i = start; i <= end; i++) {
        READ_CHECK ();
        int instr;
        if (sscanf (line, "%d", &instr) != 1)
            EXPECTED ("an integer");

        if (i >= kone->mmu_limit) {
            ELOG ("The program is too big to fit in MMU_LIMIT = %d words\n", kone->mmu_limit);
            return false;
        }
        kone->mem[kone->mmu_base + i] = instr;
    }

    // data segment
    READ_CHECK ();
    if (strcmp (line, "___data___\n"))
        EXPECTED ("___data___");

    READ_CHECK ();
    if (sscanf (line, "%d %d", &start, &end) != 2)
        EXPECTED ("two integers");

    DLOG ("Data segment: %d - %d\n", start, end);
    kone->r[SP] = end;
    ILOG ("Stack pointer set to 0x%x\n", end);

    for (int32_t i = start; i <= end; i++) {
        READ_CHECK ();
        int data;
        if (sscanf (line, "%d", &data) != 1)
            EXPECTED ("an integer");

        if (i >= kone->mmu_limit) {
            ELOG ("The program is too big to fit in MMU_LIMIT = %d words\n", kone->mmu_limit);
            return false;
        }
        kone->mem[kone->mmu_base + i] = data;
    }

    // symbol table
    READ_CHECK ();
    if (strcmp (line, "___symboltable___\n"))
        EXPECTED ("___symboltable___");

    while (true) {
        READ_CHECK ();
        if (!strcmp (line, "___end___\n"))
            break;

        char name[1024], value[1024];
        if (sscanf (line, "%s %s", name, value) != 2)
            EXPECTED ("a name-value pair");

        if (!symtable_insert (name, value))
            return false;

        DLOG ("Symbol added: %s = %s\n", name, value);
    }

    return true;
}


/**
 * Frees all memory allocated by ckone_init() and ckone_load().
 */
void 
ckone_free (
        s_ckone* kone       ///< The state structure.
        ) 
{
    symtable_clear ();

    if (kone->mem)
        free (kone->mem);

    kone->mem = NULL;
    kone->mem_size = 0;
    kone->mmu_limit = 0;
}


/**
 * @internal
 * Print the contents of the emulator memory. The number of columns
 * and the number base is determined by command line arguments and
 * a compile-time option (DEFAULT_MEMDUMP_BASE).
 */
static void 
ckone_dump_memory (
        s_ckone* kone       ///< The state structure.
        ) 
{
    int cols = args.mem_cols;

    printf ("Memory size: %u words, MMU base: 0x%08x (%d), MMU limit: %d words\n",
            kone->mem_size, kone->mmu_base, kone->mmu_base, kone->mmu_limit);
    printf ("Accessible memory area: 0x%08x - 0x%08x (%d - %d)\n",
            kone->mmu_base, kone->mmu_base + kone->mmu_limit - 1,
            kone->mmu_base, kone->mmu_base + kone->mmu_limit - 1);


    // choose the number base based on both a compile-time option
    // and a command line argument
    int base = DEFAULT_MEMDUMP_BASE;
    if (args.mem_swap_base)
        base = (base == 10)? 16 : 10;


    // table header
    printf ("Memory      ");
    for (int i = 0; i < cols; i++) {
        if (base == 10)
            printf("%12d", i);         // column offset
        else
            printf("%12x", i);
    }
    printf ("\n");

    printf ("------------");
    for (int i = 0; i < cols; i++)
        printf("------------");
    printf ("\n");

    for (int32_t i = 0; i < kone->mem_size; i++) {
        if (i % cols == 0) {
            if (base == 10)
                printf ("%10u |", i);
            else
                printf ("0x%08x |", i);
        }

        if (base == 10)
            printf (" %11d", kone->mem[i]);
        else
            printf ("  0x%08x", kone->mem[i]);

        if ((i % cols == cols - 1) || (i == kone->mem_size - 1))
            printf ("\n");
    }
}


/**
 * @internal
 * Print a number in both hexadecimal and decimal formats.
 */
static void 
print_hex_dec (
        int32_t value       ///< The value to print.
        ) 
{
    printf("0x%08x (%11d)", value, value);
}


/**
 * @internal
 * Print the contents of the registers.
 */
static void 
ckone_dump_registers (
        s_ckone* kone       ///< The state structure.
        ) 
{
    printf ("Registers:\n");
    for (e_register r = R0; r <= R7; r++) {
        if (r < 6)
            printf ("R%d      = ", r);
        else if (r == 6)
            printf ("R%d (SP) = ", r);
        else if (r == 7)
            printf ("R%d (FP) = ", r);

        print_hex_dec (kone->r[r]);
        printf ("   ");
        switch (r) {
            case R0: printf ("PC      = "); print_hex_dec (kone->pc); break;
            case R1: printf ("IR      = "); print_hex_dec (kone->ir); break;
            case R2: printf ("TR      = "); print_hex_dec (kone->tr); break;
            case R3: printf ("ALU_IN1 = "); print_hex_dec (kone->alu_in1); break;
            case R4: printf ("ALU_IN2 = "); print_hex_dec (kone->alu_in2); break;
            case R5: printf ("ALU_OUT = "); print_hex_dec (kone->alu_out); break;
            case R6: printf ("MAR     = "); print_hex_dec (kone->mar); break;
            case R7: printf ("MBR     = "); print_hex_dec (kone->mbr); break;
        }
        printf ("\n");
    }
    printf ("SR = ");

    int32_t sr = kone->sr;
    printf ("%c%c%c%c%c%c%c... (0x%08x)\n",
            (sr & SR_G)? 'G' : 'g',
            (sr & SR_E)? 'E' : 'e',
            (sr & SR_L)? 'L' : 'l',
            (sr & SR_O)? 'O' : 'o',
            (sr & SR_Z)? 'Z' : 'z',
            (sr & SR_U)? 'U' : 'u',
            (sr & SR_M)? 'M' : 'm',
            sr);
    
}


/**
 * @internal
 * Print the current state. Prints the registers, the
 * next instruction (if in stepping mode), the symbol
 * table (if enabled by a command line argument), and
 * the memory contents.
 */
static void 
ckone_dump (
        s_ckone* kone           ///< The state structure.
        ) 
{
    printf ("\nCurrent state:\n\n");
    ckone_dump_registers (kone);
    if (args.step) {
        char buf[1024];
        if (!kone->halted && kone->pc >= 0 && kone->pc < kone->mmu_limit)
            instr_string (kone->mem[kone->mmu_base + kone->pc], buf, sizeof(buf));
        else
            snprintf (buf, sizeof(buf), "N/A");
        printf ("\n>>> Next instruction: %s\n", buf);
    }
    printf ("\n");
    if (args.include_symtable) {
        symtable_dump ();
        printf ("\n");
    }
    ckone_dump_memory (kone);
    printf ("\n");
}


/**
 * @internal
 * Pause execution after an instruction. The user can either continue
 * to the next instruction, show the symbol table, or quit.
 *
 * @return True if the simulation should continue.
 */
static bool 
pause (
        void
        ) 
{
    while (true) {
        printf ("Type enter to execute the next instruction, \"s\" to show\n"
                "the symbol table, or \"q\" to quit: \n");

        char buf[1024];
        fgets (buf, sizeof (buf), stdin);

        if (!strcmp (buf, "\n"))
            return true;

        if (!strcmp (buf, "s\n")) {
            printf ("\n");
            symtable_dump ();
            printf ("\n");
        }

        if (!strcmp (buf, "q\n"))
            return false;
    }
}


/**
 * Start emulation. The emulation will run until an error occurs
 * or the CPU halts. If stepping mode is on, the emulation will pause
 * between every instruction. In this case the user can also choose
 * to stop any time the emulation has paused.
 *
 * @return EXIT_FAILURE if something went wrong, EXIT_SUCCESS otherwise.
 */
int 
ckone_run (
        s_ckone* kone       ///< The state structure.
        ) 
{
    ILOG ("Running program...\n", 0);
    if (args.step) {
        ckone_dump (kone);
        if (!pause ())
            return EXIT_FAILURE;
    }

    while (!kone->halted) {
        if (!cpu_step (kone)) {
            ILOG ("Execution stopped.\n", 0);
            ckone_dump (kone);
            return EXIT_FAILURE;
        }
        if (args.step) {
            ckone_dump (kone);
            if (!kone->halted)
                if (!pause ())
                    return EXIT_FAILURE;
        }
    }
    if (!args.step)
        ckone_dump (kone);

    return EXIT_SUCCESS;
}

