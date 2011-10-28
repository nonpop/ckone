/**
 * @file log.h
 *
 * The logging API. The xLOG macros should be used
 * for writing different kinds of logging messages.
 */

#ifndef LOG_H
#define LOG_H


/**
 * The log message types.
 */
typedef enum {
    LOG_DEBUG,      ///< A debug message. Only shown if verbosity == 2.
    LOG_INFO,       ///< An information message. Only shown if verbosity >= 1.
    LOG_WARN,       ///< A warning. Always shown.
    LOG_ERROR       ///< An error message. Always shown.
} e_loglevel;


extern void wlog (e_loglevel lvl, const char* fmt, ...);


/// Print a debug message, with the current file and line included.
#define DLOG(fmt, ...) wlog (LOG_DEBUG, "DEBUG: " __FILE__ ":%d: " fmt, __LINE__, __VA_ARGS__)

/// Print an information message.
#define ILOG(fmt, ...) wlog (LOG_INFO, "Info: " fmt, __VA_ARGS__)

/// Print a warning.
#define WLOG(fmt, ...) wlog (LOG_WARN, "Warning: " fmt, __VA_ARGS__)

/// Print an error message.
#define ELOG(fmt, ...) wlog (LOG_ERROR, "ERROR: " fmt, __VA_ARGS__)


#endif

