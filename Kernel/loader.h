#ifndef LOADER_H
#define LOADER_H

#include <stdbool.h>

typedef struct reg {
  int32_t a;
  int32_t b;
  int32_t c;
  int32_t d;
  int32 e;
} reg_t;

typedef struct tcb {
  int pid;
  int tid;
  bool km;
  int seg_base;
  int seg_size;
  uint32_t ptr_inst;
  uint32_t b_stack;
  uint32_t c_stack;
  reg_t reg;
} tcb_t;

#endif
