#include "internal.h"
#include <dlfcn.h>

// Must be configured at init-time
static const function_metadata_t *md = NULL;

void hydra_function_metadata_init(void)
{
  const function_metadata_t *(*user_fn)(void) = NULL;
  *(void**)&user_fn = dlsym(RTLD_DEFAULT, "hydra_user_functions");
  if (!user_fn) FAIL("Failed to find user metadata: hydra_user_functions()");
  md = user_fn();
}


const function_def_t * function_find(const char *name)
{
  for (size_t i = 0; i < md->n_defs; i++) {
    if (0 == strcmp(name, md->defs[i].name)) {
      return &md->defs[i];
    }
  }
  return NULL;
}

const char *function_name(segoff_t s)
{
 u32 addr = segoff_abs(s);
  for (size_t i = 0; i < md->n_defs; i++) {
    const function_def_t *f = &md->defs[i];
    if (addr == segoff_abs(f->addr)) {
      return f->name;
    }
  }
  return NULL;
}

bool function_addr(const char *name, segoff_t *_out)
{
  for (size_t i = 0; i < md->n_defs; i++) {
    const function_def_t *f = &md->defs[i];
    if (0 == strcmp(name, f->name)) {
      if (_out) *_out = f->addr;
      return true;
    }
  }
  return false;
}
