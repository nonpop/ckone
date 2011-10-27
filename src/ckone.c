#include <stdlib.h>
#include <string.h>
#include "ckone.h"
#include "args.h"
#include "cpu.h"
#include "symtable.h"
#include "instr.h"


/**
 * Initializes the ckone. Allocates memory and resets the CPU.
 * If the clean flag is set, it will also zero all memory and
 * registers.
 *
 * @return True if successful, false otherwise.
 */
bool ckone_init (s_ckone* kone) {
    if (args.clean)
        memset (kone, 0, sizeof(s_ckone));

    kone->mem = malloc (args.mem_size*sizeof(uint32_t));
    if (!kone->mem) {
        ELOG ("Cannot allocate %d bytes of memory\n", args.mem_size*sizeof(uint32_t));
        return false;
    }

    if (args.clean)
        memset (kone->mem, 0, args.mem_size*sizeof(uint32_t));

    kone->mem_size = args.mem_size;
    kone->mmu_base = args.mmu_base;
    kone->mmu_limit = args.mmu_limit;

    cpu_reset (kone);

    return true;
}


char* read_line (FILE* input, int* linenum) {
    static char buf[1024];
    if (!fgets (buf, sizeof(buf), input)) {
        ELOG ("Failed to read from program file\n", 0);
        return NULL;
    }
    (*linenum)++;
    return buf;
}


/**
 * Load a program into memory.
 *
 * @param input The input file.
 * @return True if successful, false otherwise.
 */
bool ckone_load (s_ckone* kone, FILE* input) {
    int linenum = 0;
    char* line;

    // little macros to make things easier to read
#define READ_CHECK() if (!(line = read_line (input, &linenum))) return false
#define EXPECTED(what) { \
    ELOG ("Expected " what " at line %d but got %s\n", linenum, line); \
    return false; \
}

    // header
    READ_CHECK ();
    if (strcmp (line, "___b91___\n"))
        EXPECTED ("___b91___");

    // code segment
    READ_CHECK ();
    if (strcmp (line, "___code___\n"))
        EXPECTED ("___code___");

    int start, end;
    READ_CHECK ();
    if (sscanf (line, "%d %d", &start, &end) != 2)
        EXPECTED ("two integers");
    
    DLOG ("Code segment: %d - %d\n", start, end);
    for (int i = start; i <= end; i++) {
        READ_CHECK ();
        int instr;
        if (sscanf (line, "%d", &instr) != 1)
            EXPECTED ("an integer");

        kone->mem[i] = instr;
    }

    // data segment
    READ_CHECK ();
    if (strcmp (line, "___data___\n"))
        EXPECTED ("___data___");

    READ_CHECK ();
    if (sscanf (line, "%d %d", &start, &end) != 2)
        EXPECTED ("two integers");

    DLOG ("Data segment: %d - %d\n", start, end);
    for (int i = start; i <= end; i++) {
        READ_CHECK ();
        int data;
        if (sscanf (line, "%d", &data) != 1)
            EXPECTED ("an integer");

        kone->mem[i] = data;
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


void ckone_dump_memory (s_ckone* kone) {
    unsigned int cols = args.mem_cols;

    printf ("Memory size: %d words, MMU base: 0x%08x, MMU limit: %d words\n",
            kone->mem_size, kone->mmu_base, kone->mmu_limit);
    printf ("(Accessible memory area: 0x%08x - 0x%08x)\n",
            kone->mmu_base, kone->mmu_base + kone->mmu_limit);

    // table header
    printf ("Memory      ");
    for (size_t i = 0; i < cols; i++)
        printf("%12d", i);         // column offset
    printf ("\n");

    printf("------------");
    for (size_t i = 0; i < cols; i++)
        printf("------------");
    printf ("\n");

    for (size_t i = 0; i < kone->mem_size; i++) {
        if (i % cols == 0)
            printf ("0x%08x |", i);

        printf ("  0x%08x", kone->mem[i]);
        if ((i % cols == cols - 1) || (i == kone->mem_size - 1))
            printf ("\n");
    }
}


void print_hex_dec (int32_t value) {
    printf("0x%08x (%11d)", value, value);
}


void ckone_dump_registers (s_ckone* kone) {
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


void ckone_dump (s_ckone* kone) {
    printf ("\nCurrent state:\n\n");
    ckone_dump_registers (kone);
    printf ("\n");
    if (args.include_symtable) {
        symtable_dump ();
        printf ("\n");
    }
    ckone_dump_memory (kone);
    printf ("\n");
}


int ckone_run (s_ckone* kone) {
    ILOG ("Running program...\n", 0);
    while (!kone->halted) {
        if (!cpu_step (kone)) {
            ILOG ("Execution stopped.\n", 0);
            ckone_dump (kone);
            return EXIT_FAILURE;
        }
        if (args.step) {
            ckone_dump (kone);

            if (!kone->halted) {
                fprintf (stderr, "Press enter to continue...");
                getchar ();
            }
        }
    }
    if (!args.step)
        ckone_dump (kone);

    return EXIT_SUCCESS;
}

