#include <stdarg.h>
#include <stdio.h>
#include "log.h"


e_loglevel loglevel;


void wlog (e_loglevel lvl, const char* fmt, ...) {
    if (lvl >= loglevel)
    {
        va_list ap;
        va_start (ap, fmt);
        vprintf (fmt, ap);
    }
}

