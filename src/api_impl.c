#include "internal.h"
#include <dlfcn.h>

HYDRA_MACHINE_INIT_FUNC(hydra_machine_init)
{
  hydra_function_metadata_init();
  hydra_callstack_init();
  hydra_exec_init(hw, audio);

  // User init
  void (*hydra_user_init)(hydra_conf_t *conf, hydra_machine_hardware_t *hw, hydra_machine_audio_t *audio) = NULL;
  *(void**)&hydra_user_init = dlsym(RTLD_DEFAULT, "hydra_user_init");
  if (!hydra_user_init) FAIL("Failed to find user init function: hydra_user_init()");

  hydra_user_init(HYDRA_CONF, hw, audio);

  if (!HYDRA_CONF->code_load_offset) FAIL("User init failed to set init->code_load_offset");
  if (!HYDRA_CONF->data_section_seg) FAIL("User init failed to set init->data_section_seg");

  // Set up the datasection baseptr
  u16 seg = HYDRA_CONF->code_load_offset + HYDRA_CONF->data_section_seg;
  u8 *ptr = hw->mem_hostaddr(hw->ctx, 16 * seg);
  hydra_datasection_baseptr_set(ptr);
}

HYDRA_MACHINE_EXEC_FUNC(hydra_machine_exec)
{
  if (m->registers->cs != 0xffff) {
    hydra_callstack_track(m, interrupt_count);
  }
  return hydra_exec_run(m);
}

HYDRA_MACHINE_NOTIFY_FUNC(hydra_machine_notify)
{
  hydra_callstack_notify(m);
}
