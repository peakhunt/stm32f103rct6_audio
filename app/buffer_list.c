#include "buffer_list.h"

void
buffer_list_init(buffer_list_t* bl)
{
  bl->num_free = 0;
  bl->num_used = 0;

  INIT_LIST_HEAD(&bl->free);
  INIT_LIST_HEAD(&bl->used);
}

void
buffer_list_add_free(buffer_list_t* bl, buffer_head_t* b)
{
  list_add_tail(&b->le, &bl->free);
  bl->num_free++;
}

void
buffer_list_add_used(buffer_list_t* bl, buffer_head_t* b)
{
  list_add_tail(&b->le, &bl->used);
  bl->num_used++;
}

buffer_head_t*
buffer_list_get_free(buffer_list_t* bl)
{
  buffer_head_t*    b;

  if(bl->num_free == 0)
  {
    return NULL;
  }

  b = list_first_entry(&bl->free, buffer_head_t, le);
  list_del_init(&b->le);

  return b;
}

buffer_head_t*
buffer_list_get_used(buffer_list_t* bl)
{
  buffer_head_t*    b;

  if(bl->num_used == 0)
  {
    return NULL;
  }

  b = list_first_entry(&bl->used, buffer_head_t, le);
  list_del_init(&b->le);

  return b;
}

uint32_t
buffer_list_num_free(buffer_list_t* bl)
{
  return 0;
}

uint32_t
buffer_list_num_used(buffer_list_t* bl)
{
  return 0;
}
