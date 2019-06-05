#ifndef BMOS_REG_H
#define BMOS_REG_H

typedef enum {
  BMOS_REG_TYPE_TASK,
  BMOS_REG_TYPE_QUEUE,
  BMOS_REG_TYPE_SEM,
  BMOS_REG_TYPE_MUT,
  BMOS_REG_TYPE_COUNT
} bmos_reg_type_t;

void bmos_reg(bmos_reg_type_t type, void *p);

#endif
