#include "internal.h"

static hydra_hook_t hooks[MAX_HOOKS];
static size_t num_hooks = 0;

void hydra_hook_register(hydra_hook_t ent)
{
  assert(num_hooks < ARRAY_SIZE(hooks));
  hooks[num_hooks++] = ent;
}

hydra_hook_t * hydra_hook_find(addr_t addr)
{
  for (size_t i = 0; i < num_hooks; i++) {
    hydra_hook_t *ent = &hooks[i];
    if (ent->hook_ip == addr.off && ent->hook_cs + CODE_START_SEG == addr.seg) {
      return ent;
    }
  }
  return NULL;
}

hydra_result_t hydra_hook_dead(hydra_machine_t *m)
{
  hydra_callstack_dump();
  hydra_cpu_dump(m->registers);
  FAIL("DEADCOE NOT SO DEAD at CS: %x IP: %x", m->registers->cs, m->registers->ip);
  return HYDRA_RESUME();
}
