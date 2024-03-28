
enum {
  STATE_UNINIT = 0,
  STATE_IDLE,
  STATE_ACTIVE,
  STATE_DONE,
};

typedef struct execution execution_t;
struct execution
{
  pthread_t             thread;
  pthread_mutex_t       mutex[1];
  pthread_cond_t        cond_main[1];
  pthread_cond_t        cond_child[1];
  int                   state;

  hook_entry_t *        hook;
  hooklib_machine_t     machine;
  hook_result_t         result;

  u16                   saved_cs;
  u16                   saved_ip;
};

execution_t * execution_context_get(u16 *opt_exec_id);
void          execution_context_set(execution_t *exec);

void exec_init(hooklib_machine_hardware_t *hw, hooklib_audio_t *audio);
int exec_run(hooklib_machine_t *m);
