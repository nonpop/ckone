/**
 * @file ext.h
 *
 * The public functions for external device operations.
 */

#ifndef EXT_H
#define EXT_H


extern void ext_init_devices ();
extern void ext_close_devices ();

extern void ext_in (s_ckone* kone);
extern void ext_out (s_ckone* kone);
extern int32_t ext_svc (s_ckone* kone);


#endif

