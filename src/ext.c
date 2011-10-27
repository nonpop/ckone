/**
 * @file ext.c
 *
 * Implements operations involving the external world.
 * These operations are IN, OUT, and SVC.
 *
 * Calls functions from mmu.c to read and write memory, and
 * from instr.c to decode instructions.
 */


#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ckone.h"
#include "instr.h"
#include "mmu.h"
#include "args.h"


/// @cond private
/**
 * A structure containing information about a device.
 */
typedef struct {
    int num;        ///< The device number.
    char* name;     ///< The device name.
    FILE* file;     ///< The file where the device reads/writes data from/to.
    bool is_input;  ///< True if this device is an input device, false
                    ///< if it is an output device.
} s_device;


/**
 * The available device numbers.
 */
enum e_device_number {
    CRT = 0,        ///< The display device. Always stdout.
    KBD = 1,        ///< The keyboard device. Always stdin.
    STDIN = 6,      ///< The STDIN device. The file for this can be defined
                    ///< in the program file and overridden with a command
                    ///< line argument.
    STDOUT = 7      ///< The STDOUT device. The file for this can be defined
                    ///< in the program file and overridden with a command
                    ///< line argument.
};


/**
 * The available devices. This must be initialized with
 * ext_init_devices () before starting the emulation.
 */
static s_device devices[] = {
    { 0, "CRT", NULL, false },
    { 1, "KBD", NULL, true },
    { 6, "STDIN", NULL, true },
    { 7, "STDOUT", NULL, false },
    { -1, "(Unknown)", NULL, false },
};

/// @endcond


/**
 * Initialize the external devices. CRT is will be stdout and
 * KBD will be stdin. The values in the ::args structure define
 * the STDIN and STDOUT devices. This must be called before
 * emulation is started. See also ext_close_devices ().
 */
void 
ext_init_devices (
        void
        ) 
{
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
 * Close the files for the external devices. See ext_init_devices ().
 */
void 
ext_close_devices (
        void
        ) 
{
    ILOG ("Closing external devices...\n", 0);
    if (devices[2].file)
        fclose (devices[2].file);
    if (devices[3].file)
        fclose (devices[3].file);
}


/**
 * Read an integer from the given file. If the file is stdin,
 * it also prints a prompt.
 *
 * @return The integer read.
 */
static int32_t 
read_input (
        FILE* in        ///< The input file.
        ) 
{
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
 * it also prints a prefix telling where the value came from.
 */
static void 
write_output (
        FILE* out,      ///< The output file.
        uint32_t value  ///< The value to write.
        ) 
{
    if (out == stdout)
        printf ("Program outputted: ");

    fprintf (out, "%d\n", value);
}


/**
 * Get data for the given device.
 *
 * @return The device data. NULL if the device does not exist.
 */
static s_device* 
get_device (
        int32_t dev_num     ///< The device number.
        ) 
{
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
 * @return The name of the device.
 */
static const char* 
get_device_name (
        uint32_t dev_num        ///< The device number.
        ) 
{
    s_device* dev = get_device (dev_num);
    if (!dev)
        return "(Unknown)";
    else
        return dev->name;
}


/**
 * Get the file of a device.
 *
 * @return The device file. NULL if the device does not exist or if
 *         it's of the wrong type (input vs. output).
 */
static FILE* 
get_device_file (
        uint32_t dev_num,   ///< The device number.
        bool input          ///< True if an input device is requested.
        ) 
{
    s_device* dev = get_device (dev_num);
    if (dev == NULL)
        return NULL;

    if (dev->is_input != input) {
        ELOG ("Device %s is not an %s device\n",
                dev->name, input? "input" : "output");
        return NULL;
    }

    if (dev->file == NULL)
        ELOG ("The file for device %s is NULL\n", dev->name);

    return dev->file;
}


/**
 * Read an integer from the device denoted in TR and store the
 * result in the first operand register.
 * 
 * Affects: Rx
 *
 * Affected status bits: ::SR_M
 */
void 
ext_in (
        s_ckone* kone       ///< The state structure.
        ) 
{
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
 * Affected status bits: ::SR_M
 */
void 
ext_out (
        s_ckone* kone       ///< The state structure.
        ) 
{
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
 * @return The number of arguments for this SVC, which is 0.
 *
 * Affects: halted
 */
static int32_t 
svc_halt (
        s_ckone* kone       ///< The state structure.
        ) 
{
    DLOG ("SVC HALT\n", 0);
    kone->halted = true;
    ILOG ("Halted.\n", 0);
    return 0;
}


/**
 * Read a value from KBD and store it to the location given
 * on the stack.
 *
 * @note At least TitoKone 1.203 seems to have a bug here which
 * causes READ to take two arguments and ignore the second one.
 * This can be emulated using the --emulate-bugs flag.
 *
 * @return The number of arguments for this SVC, which is 1
 *         normally, or 2 if bugs are emulated.
 *
 * Affects: MAR, MBR
 *
 * Affected status bits: ::SR_M
 */
static int32_t 
svc_read (
        s_ckone* kone       ///< The state structure.
        ) 
{
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
 * @return The number of arguments for this SVC, which is 1.
 *
 * Affects: MAR, MBR
 *
 * Affected status bits: ::SR_M
 */
static int32_t 
svc_write (
        s_ckone* kone       ///< The state structure.
        ) 
{
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
 * @return The number of arguments for this SVC, which is 3.
 *
 * Affects: MAR, MBR
 *
 * Affected status bits: ::SR_M
 */
static int32_t 
svc_time (
        s_ckone* kone       ///< The state structure.
        ) 
{
    DLOG ("SVC TIME\n", 0);
    time_t now = time (NULL);
    struct tm* t = localtime (&now);

    DLOG ("Now is: %s\n", asctime (t));

    kone->mar = kone->r[FP] - 2;    // address of seconds variable
    mmu_read (kone);
    kone->mar = kone->mbr;
    kone->mbr = t->tm_sec;
    mmu_write (kone);

    kone->mar = kone->r[FP] - 3;    // address of minutes variable
    mmu_read (kone);
    kone->mar = kone->mbr;
    kone->mbr = t->tm_min;
    mmu_write (kone);

    kone->mar = kone->r[FP] - 4;    // address of hours variable
    mmu_read (kone);
    kone->mar = kone->mbr;
    kone->mbr = t->tm_hour;
    mmu_write (kone);

    return 3;
}


/**
 * Get the current date and store it to the locations given on the stack.
 *
 * NOTE: At least TitoKone 1.203 seems to report the month as one too small.
 * This can be replicated using the --emulate-bugs flag.
 *
 * @return How The number of arguments for this SVC, which is 3.
 *
 * Affects: MAR, MBR
 *
 * Affected status bits: ::SR_M
 */
static int32_t 
svc_date (
        s_ckone* kone       ///< The state structure.
        ) 
{
    DLOG ("SVC DATE\n", 0);
    time_t now = time (NULL);
    struct tm* t = localtime (&now);

    DLOG ("Now is: %s\n", asctime (t));

    kone->mar = kone->r[FP] - 2;        // address of days variable
    mmu_read (kone);
    kone->mar = kone->mbr;
    kone->mbr = t->tm_mday;
    mmu_write (kone);

    kone->mar = kone->r[FP] - 3;        // address of months variable
    mmu_read (kone);
    kone->mar = kone->mbr;
    kone->mbr = t->tm_mon + (args.emulate_bugs? 0 : 1);
    mmu_write (kone);

    kone->mar = kone->r[FP] - 4;        // address of years variable
    mmu_read (kone);
    kone->mar = kone->mbr;
    kone->mbr = t->tm_year + 1900;
    mmu_write (kone);

    return 3;
}


/**
 * Execute an svc command.
 *
 * @return The number of arguments for the SVC.
 *
 * Affects:
 *  - halted (HALT)
 *  - MAR, MBR (the rest)
 *
 * Affected status bits: ::SR_M (not HALT)
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

