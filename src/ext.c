#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ckone.h"
#include "instr.h"
#include "mmu.h"
#include "args.h"


typedef struct {
    int num;
    char* name;
    FILE* file;
    bool is_input;
} s_device;


enum {
    CRT = 0, KBD = 1, STDIN = 6, STDOUT = 7
};


static s_device devices[] = {
    { 0, "CRT", NULL, false },
    { 1, "KBD", NULL, true },
    { 6, "STDIN", NULL, true },
    { 7, "STDOUT", NULL, false },
    { -1, "(Unknown)", NULL, false },
};


/**
 * Initialize the external devices. CRT is will be stdout and
 * KBD will be stdin. The values in the args structure define
 * the STDIN and STDOUT devices.
 */
void ext_init_devices () {
    ILOG ("Initializing external devices...\n", 0);
    devices[0].file = stdout;
    devices[1].file = stdin;

    if (!args.stdin_file)
        args.stdin_file = "stdin";
    if (!args.stdout_file)
        args.stdout_file = "stdout";

    ILOG ("Opening STDIN file: %s\n", args.stdin_file);

    devices[2].file = fopen (args.stdin_file, "r");
    if (!devices[2].file)
        WLOG ("Cannot open %s for reading; trying to read from STDIN will not work\n",
                args.stdin_file);
    
    ILOG ("Opening STDOUT file: %s\n", args.stdout_file);

    devices[3].file = fopen (args.stdout_file, "w");
    if (!devices[3].file)
        WLOG ("Cannot open %s for writing; trying to write to STDOUT will not work\n",
                args.stdout_file);
}


/**
 * Read an integer from the given file. If the file is stdin,
 * it also prints a prompt.
 *
 * @param in The input file.
 * @return The integer read.
 */
static int32_t read_input (FILE* in) {
    if (in == stdin)
        printf ("Enter an integer: ");

    // read an integer and make sure a whole line is consumed
    char buf[32];
    memset (buf, 0, sizeof(buf));
    for (unsigned int i = 0; ; i++) {
        int c = fgetc (in);
        if (c == EOF || c == '\n')
            break;

        if (i < sizeof(buf) - 1)
            buf[i] = c;
    }

    int32_t value;
    if (sscanf (buf, "%d", &value) != 1)
        WLOG ("The value read was not an integer.\n", 0);
        
    return value;
}


/**
 * Write an integer to the given file. If the file is stdout,
 * it also prints a prefix thing.
 *
 * @param out The output file.
 * @param value The value to write.
 */
static void write_output (FILE* out, uint32_t value) {
    if (out == stdout)
        printf ("Program outputted: ");

    fprintf (out, "%d\n", value);
}


/**
 * Get data for the given device.
 *
 * @param dev_num The device number.
 * @return The device data. NULL if the device does not exist.
 */
static s_device* get_device (int32_t dev_num) {
    DLOG ("Finding device %d...\n", dev_num);

    for (unsigned int i = 0; i < sizeof(devices)/sizeof(s_device); i++) {
        if (devices[i].num == -1) {
            ELOG ("Device %d does not exist\n", dev_num);
            break;
        } else if (devices[i].num == dev_num)
            return &devices[i];
    }
    return NULL;
}


/**
 * Get the name of a device.
 *
 * @param dev_num The device number.
 * @return The name of the device.
 */
static const char* get_device_name (uint32_t dev_num) {
    s_device* dev = get_device (dev_num);
    if (!dev)
        return "(Unknown)";
    else
        return dev->name;
}


/**
 * Get the file of a device.
 *
 * @param dev_num The device number.
 * @param input Whether the device should be an input device or not.
 * @return The device file. NULL if the device does not exist or if
 * it's of the wrong type (input vs. output)
 */
static FILE* get_device_file (uint32_t dev_num, bool input) {
    s_device* dev = get_device (dev_num);
    if (dev == NULL)
        return NULL;

    if (dev->is_input != input) {
        ELOG ("Device %d is not an %s device\n",
                dev_num, input? "input" : "output");
        return NULL;
    }

    if (dev->file == NULL)
        ELOG ("The file for device %d is NULL\n", dev_num);

    return dev->file;
}


/**
 * Read an integer from the device denoted in TR and store the
 * result in the first operand register.
 * 
 * Affects: Rx
 * Status: M (invalid device)
 */
void ext_in (s_ckone* kone) {
    DLOG ("Reading input from device %d...\n", kone->tr);

    FILE* f = get_device_file (kone->tr, true);
    if (!f) {
        kone->sr |= SR_M;
        return;
    }

    int32_t value = read_input (f);
    kone->r[instr_first_operand (kone->ir)] = value;

    DLOG ("Read %d from %s\n", value, get_device_name (kone->tr));
}


/**
 * Writes the value in the current instruction's first operand
 * register to the device denoted in TR.
 *
 * Status: M (invalid device)
 */
void ext_out (s_ckone* kone) {
    DLOG ("Writing output to device %d...\n", kone->tr);

    FILE* f = get_device_file (kone->tr, false);
    if (!f) {
        kone->sr |= SR_M;
        return;
    }

    int32_t value = kone->r[instr_first_operand (kone->ir)];
    write_output (f, value);

    DLOG ("Wrote %d to %s\n", value, get_device_name (kone->tr));
}


/**
 * Halts the machine.
 *
 * Affects: halted
 */
static int32_t svc_halt (s_ckone* kone) {
    DLOG ("SVC HALT\n", 0);
    kone->halted = true;
    ILOG ("Halted.\n", 0);
    return 0;
}


/**
 * Read a value from KBD and store it to the location given
 * on the stack.
 *
 * NOTE: At least TitoKone 1.203 seems to have a bug here which
 * causes READ to take two arguments and ignore the second one.
 * This can be emulated using the --emulate-bugs flag.
 *
 * @return How many arguments to pop.
 *
 * Affects: MAR, MBR
 * Status: M (invalid memory access)
 */
static int32_t svc_read (s_ckone* kone) {
    DLOG ("SVC READ\n", 0);
    FILE* f = get_device_file (KBD, true);
    if (!f) {
        ELOG ("WTF?", 0);
        return 0;
    }

    uint32_t ofs = args.emulate_bugs? 1 : 0;

    kone->mar = kone->r[FP] - (2 + ofs);
    mmu_read (kone);    // read the address of the destination variable
    DLOG ("Destination: 0x%x\n", kone->mbr);
    kone->mar = kone->mbr;
    kone->mbr = read_input (f);     // read the value from keyboard
    DLOG ("Read %d from KBD\n", kone->mbr);
    mmu_write (kone);               // write it to the destination variable

    return 1 + ofs;
}


/**
 * Write a value given on the stack to CRT.
 *
 * @return How many arguments to pop.
 *
 * Affects: MAR, MBR
 * Status: M (invalid memory access)
 */
static int32_t svc_write (s_ckone* kone) {
    DLOG ("SVC WRITE\n", 0);
    FILE* f = get_device_file (CRT, false);
    if (!f) {
        ELOG ("WTF?", 0);
        return 0;
    }

    kone->mar = kone->r[FP] - 2;
    mmu_read (kone);
    write_output (f, kone->mbr);
    DLOG ("Wrote %d to CRT\n", kone->mbr);
    return 1;
}


/**
 * Get the current time and store it to the locations given on the stack.
 *
 * @return How many arguments to pop.
 *
 * Affects: MAR, MBR
 * Status: M (invalid memory access)
 */
static int32_t svc_time (s_ckone* kone) {
    DLOG ("SVC TIME\n", 0);
    time_t now = time (NULL);
    struct tm* t = localtime (&now);

    DLOG ("Now is: %s\n", asctime (t));

    kone->mar = kone->r[FP] - 2;
    kone->mbr = t->tm_sec;
    mmu_write (kone);
    kone->mar--;
    kone->mbr = t->tm_min;
    mmu_write (kone);
    kone->mar--;
    kone->mbr = t->tm_hour;
    mmu_write (kone);

    return 3;
}


/**
 * Get the current date and store it to the locations given on the stack.
 *
 * @return How many arguments to pop.
 *
 * Affects: MAR, MBR
 * Status: M (invalid memory access)
 */
static int32_t svc_date (s_ckone* kone) {
    DLOG ("SVC DATE\n", 0);
    time_t now = time (NULL);
    struct tm* t = localtime (&now);

    DLOG ("Now is: %s\n", asctime (t));

    kone->mar = kone->r[FP] - 2;
    kone->mbr = t->tm_mday;
    mmu_write (kone);
    kone->mar--;
    kone->mbr = t->tm_mon + 1;
    mmu_write (kone);
    kone->mar--;
    kone->mbr = t->tm_year + 1900;
    mmu_write (kone);

    return 3;
}


/**
 * Execute an svc command.
 *
 * @return How many arguments to pop.
 *
 * Affects: Rx, MAR, MBR, halted
 * Status: M (invalid memory access, invalid device)
 */
int32_t ext_svc (s_ckone* kone) {
    switch (kone->tr) {
        case 11: return svc_halt (kone);
        case 12: return svc_read (kone);
        case 13: return svc_write (kone);
        case 14: return svc_time (kone);
        case 15: return svc_date (kone);
        default:
                 ELOG ("Invalid SVC: %d\n", kone->tr);
                 return 0;
    }
}

