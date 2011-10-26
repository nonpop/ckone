#include <stdarg.h>
#include <stdio.h>
#include "log.h"
#include "args.h"


void wlog (e_loglevel lvl, const char* fmt, ...) {
    if (lvl >= LOG_INFO || args.verbose)
    {
        va_list ap;
        va_start (ap, fmt);
        vfprintf (stderr, fmt, ap);
    }
}

