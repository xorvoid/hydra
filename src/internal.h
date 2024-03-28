#pragma once
#include "../dosbox-x/include/export/dosbox-x/hooklib.h"
#include <errno.h>
#include <pthread.h>
#include "header.h"
#include "segoff.h"
#include "hooks.h"
#include "callstack.h"
#include "machine.h"

// FIXME: REMOVE THIS HARDCODING
#define CODE_START_SEG ((u16)0x823)

#define ENABLE_DEBUG_CALLSTACK 0
#define MAX_HOOKS 2048

enum {
  HYDRA_EXEC_STATE_UNINIT = 0,
  HYDRA_EXEC_STATE_IDLE,
  HYDRA_EXEC_STATE_ACTIVE,
  HYDRA_EXEC_STATE_DONE,
};

typedef struct hydra_exec_ctx hydra_exec_ctx_t;
struct hydra_exec_ctx
{
  pthread_t             thread;
  pthread_mutex_t       mutex[1];
  pthread_cond_t        cond_main[1];
  pthread_cond_t        cond_child[1];
  int                   state;  // HYDRA_EXEC_STATE_*

  hydra_hook_entry_t *        hook;
  hooklib_machine_t     machine;
  hydra_hook_result_t         result;

  u16                   saved_cs;
  u16                   saved_ip;
};

hydra_exec_ctx_t * execution_context_get(u16 *opt_exec_id);
void               execution_context_set(hydra_exec_ctx_t *ctx);

void hydra_exec_init(hooklib_machine_hardware_t *hw, hooklib_audio_t *audio);
int hydra_exec_run(hooklib_machine_t *m);
