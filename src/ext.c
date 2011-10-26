#include <stdio.h>
#include <string.h>
#include "ckone.h"
#include "instr.h"


typedef struct {
    int num;
    char* name;
    FILE* file;
    bool is_input;
} s_device;


s_device devices[] = {
    { 0, "CRT", NULL, false },
    { 1, "KBD", NULL, true },
    { 6, "STDIN", NULL, true },
    { 7, "STDOUT", NULL, false },
    { -1, "(Unknown)", NULL, false },
};


/**
 * Initialize the external devices. CRT is will be stdout and
 * KBD will be stdin. The given arguments define the STDIN and
 * STDOUT devices.
 *
 * @param stdinf File for the STDIN device.
 * @param stdoutf File for the STDOUT device.
 */
void ext_init_devices (FILE* stdinf, FILE* stdoutf) {
    devices[0].file = stdin;
    devices[1].file = stdout;
    devices[2].file = stdinf;
    devices[3].file = stdoutf;
}


/**
 * Read an integer from the given file. If the file is stdin,
 * it also prints a prompt.
 *
 * @param in The input file.
 * @return The integer read.
 */
int32_t read_input (FILE* in) {
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
    sscanf (buf, "%d", &value);   // TODO: make sure the input really was an integer
    return value;
}


/**
 * Write an integer to the given file. If the file is stdout,
 * it also prints a prefix thing.
 *
 * @param out The output file.
 * @param value The value to write.
 */
void write_output (FILE* out, uint32_t value) {
    if (out == stdout)
        printf ("Program output: ");

    fprintf (out, "%d\n", value);
}


/**
 * Get data for the given device.
 *
 * @param dev_num The device number.
 * @return The device data. NULL if the device does not exist.
 */
s_device* get_device (int32_t dev_num) {
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
const char* get_device_name (uint32_t dev_num) {
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
FILE* get_device_file (uint32_t dev_num, bool input) {
    s_device* dev = get_device (dev_num);
    if (dev == NULL)
        return NULL;

    if (dev->is_input != input) {
        ELOG ("Device %d is not an %s device\n",
                dev_num, input? "input" : "output");
        return NULL;
    }

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
 * Affects: output
 * Status: M (invalid device)
 */
void ext_out (s_ckone* kone) {
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
int32_t svc_halt (s_ckone* kone) {
    ILOG ("Halted.\n", 0);
    kone->halted = true;
    return 0;
}


int32_t svc_read (s_ckone* kone) {
    DLOG ("svc_read ()\n", kone);
    return 1;
}


int32_t svc_write (s_ckone* kone) {
    DLOG ("svc_write ()\n", kone);
    return 1;
}


int32_t svc_time (s_ckone* kone) {
    DLOG ("svc_time ()\n", kone);
    return 3;
}


int32_t svc_date (s_ckone* kone) {
    DLOG ("svc_date ()\n", kone);
    return 3;
}


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

