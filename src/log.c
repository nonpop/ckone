/**
 * @file log.c
 *
 * A simple logger.
 */

#include <stdarg.h>
#include "common.h"
#include "args.h"


/**
 * Write the given data to stderr, if the message is important
 * enough compared to the current verbosity level.
 */
void 
wlog (
        e_loglevel lvl,         ///< The type of the message.
        const char* fmt,        ///< The format string and data (fed to vfprintf).
        ...
        ) 
{
    if ((lvl >= LOG_WARN) ||
        (lvl >= LOG_INFO && args.verbosity >= 1) ||
        (args.verbosity >= 2))
    {
        va_list ap;
        va_start (ap, fmt);
        vfprintf (stderr, fmt, ap);
    }
}

