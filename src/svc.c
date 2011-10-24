#include "ckone.h"


void svc_halt (s_ckone* kone) {
    DLOG ("svc_halt ()\n", kone);
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


void svc_call (s_ckone* kone) {
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

