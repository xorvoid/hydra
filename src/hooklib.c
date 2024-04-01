#include "internal.h"
#include <dlfcn.h>

void hydra_cpu_dump(hooklib_machine_registers_t *cpu)
{
  printf("CPU STATE:\n");
  printf("  AX: %04x  BX: %04x  CX: %04x  DX: %04x\n", cpu->ax, cpu->bx, cpu->cx, cpu->dx);
  printf("  SI: %04x  DI: %04x  BP: %04x  SP: %04x  IP: %04x\n", cpu->si, cpu->di, cpu->bp, cpu->sp, cpu->ip);
  printf("  CS: %04x  DS: %04x  ES: %04x  SS: %04x\n", cpu->cs, cpu->ds, cpu->es, cpu->ss);
  printf("  FLAGS: %04x\n", cpu->flags);
}


HOOKLIB_INIT_FUNC(hooklib_init)
{
  hydra_function_metadata_init();
  hydra_callstack_init();
  hydra_exec_init(hw, audio);

  // User init
  void (*hydra_user_init)(hydra_conf_t *conf, hooklib_machine_hardware_t *hw, hooklib_audio_t *audio) = NULL;
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

HOOKLIB_EXEC_FUNC(hooklib_exec)
{
  if (m->registers->cs != 0xffff) {
    hydra_callstack_track(m, interrupt_count);
  }
  return hydra_exec_run(m);
}

HOOKLIB_NOTIFY_FUNC(hooklib_notify)
{
  hydra_callstack_notify(m);
}
