#ifndef __BUFFERED_LIST_DEF_H__
#define __BUFFERED_LIST_DEF_H__

#include "stm32f1xx_hal.h"
#include "generic_list.h"

typedef struct
{
  struct list_head    le;
} buffer_head_t;

typedef struct
{
  uint32_t            num_free;
  uint32_t            num_used;
  struct list_head    free;
  struct list_head    used;
} buffer_list_t;

extern void buffer_list_init(buffer_list_t* bl);

extern void buffer_list_add_free(buffer_list_t* bl, buffer_head_t* b);
extern void buffer_list_add_used(buffer_list_t* bl, buffer_head_t* b);
extern buffer_head_t* buffer_list_get_free(buffer_list_t* bl);
extern buffer_head_t* buffer_list_get_used(buffer_list_t* bl);

extern uint32_t buffer_list_num_free(buffer_list_t* bl);
extern uint32_t buffer_list_num_used(buffer_list_t* bl);

static inline void
buffer_list_head_init(buffer_head_t* b)
{
  INIT_LIST_HEAD(&b->le);
}

#endif /* !__BUFFERED_LIST_DEF_H__ */
