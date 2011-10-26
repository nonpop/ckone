#include <stdio.h>
#include <string.h>
#include "ckone.h"
#include "instr.h"


/**
 * Convert a device's number to its name.
 *
 * @param dev_number the device number.
 * @return The device name.
 */
char* dev_name (int32_t dev_number) {
    switch (dev_number) {
        case 0: return "CRT";
        case 1: return "KBD";
        default: return "(Unknown)";
    }
}


/**
 * Read an integer from dev_in and store it to the input field.
 * If dev_in is NULL, the input field remains untouched.
 * 
 * Affects: input
 */
void read_input (s_ckone* kone) {
    if (kone->dev_in) {
        if (kone->dev_in == stdin)
            printf ("Enter an integer: ");

        // read an integer and make sure a whole line is consumed
        char buf[32];
        memset (buf, 0, sizeof(buf));
        for (unsigned int i = 0; ; i++) {
            int c = fgetc (kone->dev_in);
            if (c == EOF || c == '\n')
                break;

            if (i < sizeof(buf) - 1)
                buf[i] = c;
        }

        sscanf (buf, "%d", &kone->input);   // TODO: make sure the input really was an integer
    }
}


/**
 * Write the integer in the output field to dev_out. If dev_out is NULL,
 * do nothing.
 */
void write_output (s_ckone* kone) {
    if (kone->dev_out) {
        if (kone->dev_out == stdout)
            printf ("Program output: ");

        fprintf (kone->dev_out, "%d\n", kone->output);
    }
}


/**
 * Read an integer from the device denoted in TR (must be 1) and
 * store the result in the first operand register.
 * 
 * Affects: Rx, input
 * Status: M (invalid device)
 */
void ext_in (s_ckone* kone) {
    if (kone->tr != 1) {
        ELOG ("Invalid input device: %d\n", kone->tr);
        kone->sr |= SR_M;
        return;
    }

    read_input (kone);
    kone->r[instr_first_operand (kone->ir)] = kone->input;

    DLOG ("Read %d from %s\n", kone->input, dev_name (kone->tr));
}


/**
 * Writes the value in the current instruction's first operand
 * register to the device denoted in TR (must be 0).
 *
 * Affects: output
 * Status: M (invalid device)
 */
void ext_out (s_ckone* kone) {
    if (kone->tr != 0) {
        ELOG ("Invalid output device: %d\n", kone->tr);
        kone->sr |= SR_M;
        return;
    }

    kone->output = kone->r[instr_first_operand (kone->ir)];
    write_output (kone);

    DLOG ("Wrote %d to %s\n", kone->output, dev_name (kone->tr));
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

