#include "internal.h"
//#include "gizmo/gizmo.h"
//#include "gizmo/hw_native.h"
//#include "hw_dosbox.h"
//#include "data_section.h"
//#include "functions.h"

#define MAX_EXEC 1024
static hydra_exec_ctx_t executions[MAX_EXEC];
static hydra_exec_ctx_t *thread_active = NULL;

hydra_exec_ctx_t *execution_context_get(u16 *opt_exec_id)
{
  hydra_exec_ctx_t *exec = thread_active;
  if (opt_exec_id) *opt_exec_id = exec - &executions[0];
  return exec;
}

void execution_context_set(hydra_exec_ctx_t *exec)
{
  thread_active = exec;
}

static void *thread_func(void *_usr)
{
  hydra_exec_ctx_t *exec = (hydra_exec_ctx_t*)_usr;
  pthread_mutex_lock(exec->mutex);
  thread_active = exec;

  exec->result = exec->hook->func(&exec->machine);
  exec->state = HYDRA_EXEC_STATE_DONE;

  pthread_cond_signal(exec->cond_main);
  pthread_mutex_unlock(exec->mutex);

  return NULL;
}

static void execution_init(hydra_exec_ctx_t *exec)
{
  if (exec->state != HYDRA_EXEC_STATE_UNINIT) return;

  int ret = pthread_mutex_init(exec->mutex, NULL);
  if (ret != 0) FAIL("Failed to init mutex");

  ret = pthread_cond_init(exec->cond_main, NULL);
  if (ret != 0) FAIL("Failed to init cond");

  ret = pthread_cond_init(exec->cond_child, NULL);
  if (ret != 0) FAIL("Failed to init cond");
}

static hydra_exec_ctx_t *execution_acquire(void)
{
  for (size_t i = 0; i < ARRAY_SIZE(executions); i++) {
    hydra_exec_ctx_t *exec = &executions[i];
    if (exec->state > HYDRA_EXEC_STATE_IDLE) continue;

    execution_init(exec);
    exec->state = HYDRA_EXEC_STATE_ACTIVE;
    return exec;
  }
  FAIL("Reached MAX_EXEC");
}

static void execution_release(hydra_exec_ctx_t *exec)
{
  assert(exec->state != HYDRA_EXEC_STATE_IDLE);
  exec->state = HYDRA_EXEC_STATE_IDLE;
}

static hydra_hook_result_t run_wait(hydra_exec_ctx_t *exec, hooklib_machine_t *m)
{
  int ret = pthread_cond_wait(exec->cond_main, exec->mutex);
  if (ret != 0) FAIL("Failed to cond wait");

  hydra_hook_result_t result = exec->result;
  pthread_mutex_unlock(exec->mutex);

  memcpy(m, &exec->machine, sizeof(*m));

  if (exec->state == HYDRA_EXEC_STATE_DONE) {
    pthread_join(exec->thread, NULL);
    execution_release(exec);
  }

  return result;
}

static hydra_hook_result_t run_begin(hydra_hook_entry_t *hook, hooklib_machine_t *m)
{
  if (hook->flags & HYDRA_HOOK_ENTRY_FLAGS_OVERLAY) {
    // On first entry to the overlay, it calls an interrupt "int 0x3f"
    // to page in the segment. We want to allow this to happen. After the
    // overlay pager is done, it modified the code to a far-jump and returns
    // directly to it. This jump redirectly the cpu into the proper
    // segment where execution continues.

    u32 addr = (u32)m->registers->cs * 16 + m->registers->ip;
    u8 * mem = m->hardware->mem_hostaddr(m->hardware->ctx, addr);

    // "int 0x3f" is encoded as "cd 3f"
    if (0 == memcmp(mem, "\xcd\x3f", 2)) {
      printf("Call to %04x:%04x but it's not paged in.. waiting..\n", m->registers->cs - CODE_START_SEG, m->registers->ip);
      return HOOK_RESUME();
    }

    if (mem[0] != 0xea) {
      FAIL("Expected a Jump Far, found: 0x%02x", mem[0]);
    }

    u16 off, seg;
    memcpy(&off, mem+1, 2);
    memcpy(&seg, mem+3, 2);

    printf("Call to %04x:%04x paged into %04x:%04x\n", m->registers->cs - CODE_START_SEG, m->registers->ip, seg, off);

    m->registers->cs = seg;
    m->registers->ip = off;
  }

  hydra_exec_ctx_t *exec = execution_acquire();

  pthread_mutex_lock(exec->mutex);

  exec->hook = hook;
  memcpy(&exec->machine, m, sizeof(*m));

  int ret = pthread_create(&exec->thread, NULL, thread_func, exec);
  if (ret != 0) FAIL("Failed to create thread");

  return run_wait(exec, m);
}

static hydra_hook_result_t run_continue(hooklib_machine_t *m, hydra_exec_ctx_t *exec)
{
  assert(exec->state == HYDRA_EXEC_STATE_ACTIVE);

  pthread_mutex_lock(exec->mutex);
  memcpy(&exec->machine, m, sizeof(*m));

  pthread_cond_signal(exec->cond_child);

  return run_wait(exec, m);
}

static bool try_resume(hooklib_machine_t *m, hydra_hook_result_t *_result)
{
  // Resume a retf ?
  if (m->registers->cs == 0xffff) {
    hydra_exec_ctx_t *exec = &executions[m->registers->ip];
    *_result = run_continue(m, exec);
    return true;
  }

  // Resume a ret ?
  if (m->registers->cs != 0xf000 && m->registers->ip >= 0xff00) {
    size_t idx = m->registers->ip & 0xff;
    hydra_exec_ctx_t *exec = &executions[idx];
    if (m->registers->cs != exec->saved_cs) FAIL("Expected matching code segments");
    *_result = run_continue(m, exec);
    return true;
  }

  return false;
}

void hydra_exec_init(hooklib_machine_hardware_t *hw, hooklib_audio_t *audio)
{
}

int hydra_exec_run(hooklib_machine_t *m)
{
  hydra_hook_result_t result = HOOK_RESUME();

  /* printf("Hook run | CS:IP = %04x:%04x\n", */
  /*        m->registers->cs - CODE_START_SEG, m->registers->ip); */

  // Try to resume an active execution ("ret" or "retf")
  bool resumed = try_resume(m, &result);

  // Begin a new hook?
  if (!resumed) {
    segoff_t s = {
      .seg = m->registers->cs,
      .off = m->registers->ip,
    };
    hydra_hook_entry_t *ent = hydra_hook_find(s);
    if (ent) {
      result = run_begin(ent, m);
    }
  }

  // Figure out how to update / re-direct the CS:IP
  switch (result.type) {
    case HYDRA_HOOK_RESULT_TYPE_RESUME: {
      return 0;
    } break;
    case HYDRA_HOOK_RESULT_TYPE_JUMP: {
      m->registers->cs = result.new_cs + CODE_START_SEG;
      m->registers->ip = result.new_ip;
      return 1;
    } break;
    case HYDRA_HOOK_RESULT_TYPE_JUMP_NEAR: {
      m->registers->ip = result.new_ip;
      return 1;
    } break;
    case HYDRA_HOOK_RESULT_TYPE_CALL: {
      m->registers->cs = result.new_cs + CODE_START_SEG;
      m->registers->ip = result.new_ip;
      u32 addr = (u32)m->registers->ss * 16 + m->registers->sp;
      u16 ret_seg = m->hardware->mem_read16(m->hardware->ctx, addr + 2);
      u16 ret_off = m->hardware->mem_read16(m->hardware->ctx, addr + 0);
      callstack_trigger_enter(ret_seg, ret_off);
      return 1;
    } break;
    case HYDRA_HOOK_RESULT_TYPE_CALL_NEAR: {
      m->registers->ip = result.new_ip;
      u32 addr = (u32)m->registers->ss * 16 + m->registers->sp;
      u16 ret_off = m->hardware->mem_read16(m->hardware->ctx, addr + 0);
      callstack_trigger_enter(m->registers->cs, ret_off);
      return 1;
    } break;
    case HYDRA_HOOK_RESULT_TYPE_RET_NEAR: {
      u32 addr = (u32)m->registers->ss * 16 + m->registers->sp;
      m->registers->ip = m->hardware->mem_read16(m->hardware->ctx, addr + 0);
      m->registers->sp += 2;
      callstack_ret(m);
      return 1;
    } break;
      /* case HYDRA_HOOK_RESULT_TYPE_RET_FAR: { */
      /*   u32 top_of_stack = 16 * (u32)cpu->ss + (u32)cpu->sp; */
      /*   cpu->ip = m->hardware->mem_read16(m, top_of_stack); */
      /*   cpu->cs = m->hardware->mem_read16(m, top_of_stack+2); */
      /*   cpu->sp += 4; */
      /*   return 1; */
      /* } break; */
    default:
      FAIL("UNKNOWN HOOK RESULT TYPE: %d", result.type);
  }
}
