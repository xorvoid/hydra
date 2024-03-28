
void callstack_init(void);
void callstack_trigger_enter(uint16_t seg, uint16_t off);
void callstack_track(hooklib_machine_t *m, size_t interrupt_count);
void callstack_notify(hooklib_machine_t *m);
void callstack_ret(hooklib_machine_t *m);
