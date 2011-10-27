#include <stdarg.h>
#include <stdio.h>
#include "log.h"
#include "args.h"


void wlog (e_loglevel lvl, const char* fmt, ...) {
    if ((lvl >= LOG_WARN) ||
        (lvl >= LOG_INFO && args.verbosity >= 1) ||
        (args.verbosity >= 2))
    {
        va_list ap;
        va_start (ap, fmt);
        vfprintf (stderr, fmt, ap);
    }
}

