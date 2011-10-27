#ifndef LOG_H
#define LOG_H


typedef enum {
    LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FORCED_INFO
} e_loglevel;


extern void wlog (e_loglevel lvl, const char* fmt, ...);


#define DLOG(fmt, ...) wlog (LOG_DEBUG, "DEBUG: " __FILE__ ":%d: " fmt, __LINE__, __VA_ARGS__)
#define ILOG(fmt, ...) wlog (LOG_INFO, "Info: " fmt, __VA_ARGS__)
#define WLOG(fmt, ...) wlog (LOG_WARN, "Warning: " fmt, __VA_ARGS__)
#define ELOG(fmt, ...) wlog (LOG_ERROR, "ERROR: " fmt, __VA_ARGS__)
#define FILOG(fmt, ...) wlog (LOG_FORCED_INFO, "Info: " fmt, __VA_ARGS__)

#endif

