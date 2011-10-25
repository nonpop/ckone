#include <stdio.h>
#include "ckone.h"


bool ext_input (int32_t dev, int32_t* result) {
    if (dev != 1) {
        ELOG ("Invalid input device: %d\n", dev);
        return false;
    }

    printf ("Enter an integer: ");
    scanf ("%d", result);
    DLOG ("Read %d from keyboard\n", *result);
    
    return true;
}


bool ext_output (int32_t dev, int32_t value) {
    if (dev != 0) {
        ELOG ("Invalid output device: %d\n", dev);
        return false;
    }

    printf ("Program output: %d\n", value);

    return true;
}


void svc_halt (s_ckone* kone) {
    ILOG ("Halted.\n", 0);
    kone->halted = true;
}


void svc_read (s_ckone* kone) {
    DLOG ("svc_read ()\n", kone);
}


void svc_write (s_ckone* kone) {
    DLOG ("svc_write ()\n", kone);
}


void svc_time (s_ckone* kone) {
    DLOG ("svc_time ()\n", kone);
}


void svc_date (s_ckone* kone) {
    DLOG ("svc_date ()\n", kone);
}


void ext_svc (s_ckone* kone) {
    switch (kone->tr) {
        case 11: svc_halt (kone); break;
        case 12: svc_read (kone); break;
        case 13: svc_write (kone); break;
        case 14: svc_time (kone); break;
        case 15: svc_date (kone); break;
        default:
                 ELOG ("Invalid SVC: %d\n", kone->tr);
                 return;
    }
}

