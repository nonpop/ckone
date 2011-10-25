#ifndef EXT_H
#define EXT_H


#include "ckone.h"


extern void ext_svc (s_ckone* kone);
extern bool ext_input (int32_t dev, int32_t* result);
extern bool ext_output (int32_t dev, int32_t value);


#endif

