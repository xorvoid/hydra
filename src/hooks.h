
typedef struct hook_entry hook_entry_t;
typedef struct hook_result hook_result_t;

enum {
  HOOK_RESULT_TYPE_RESUME,
  HOOK_RESULT_TYPE_JUMP,
  HOOK_RESULT_TYPE_JUMP_NEAR,
  HOOK_RESULT_TYPE_CALL,
  HOOK_RESULT_TYPE_CALL_NEAR,
  HOOK_RESULT_TYPE_RET_NEAR,
  /* HOOK_RESULT_TYPE_RET_FAR, */
};

struct hook_result
{
  int type;
  uint16_t new_cs;
  uint16_t new_ip;
};

enum {
  HOOK_ENTRY_FLAGS_OVERLAY = 1<<0,
};

struct hook_entry
{
  hook_result_t (*func)(hooklib_machine_t *);
  uint16_t hook_cs, hook_ip;
  int flags;
};

void __hook_register(hook_entry_t entry);

// OLD
#define HOOK_REGISTER(func, seg, off, flags) do {  \
    hook_entry_t ent = {(func), (seg), (off), (flags)}; \
  __hook_register(ent);                      \
} while(0)

// NEW
#define HOOK_REG(name, flags) do {  \
    const char *f_name = "F_" #name; \
    hook_result_t (*h_func)(hooklib_machine_t *) = H_ ## name; \
    const funcdef_t *def = function_find(f_name);                      \
    if (!def) FAIL("Cannot find function '%s' to register", f_name); \
    hook_entry_t ent = {h_func, def->addr.seg, def->addr.off, (flags)}; \
  __hook_register(ent);                      \
} while(0)

hook_result_t H_DEAD(hooklib_machine_t *m);
#define HOOK_DEAD(func, seg, off, flags) HOOK_REGISTER(H_DEAD, seg, off, flags)

#define HOOK_FUNC(name) hook_result_t name(hooklib_machine_t *m)

#define HOOK_RESUME() ({ hook_result_t res = {HOOK_RESULT_TYPE_RESUME, -1, -1}; res; })
#define HOOK_JUMP(seg, off) ({ hook_result_t res = {HOOK_RESULT_TYPE_JUMP, seg, off}; res; })
#define HOOK_JUMP_NEAR(off) ({ hook_result_t res = {HOOK_RESULT_TYPE_JUMP_NEAR, 0, off}; res; })

// JUMP TO WHERE A NEAR OR FAR RET IS (LOL)
// FOR SOME REASON WE CAN SEEM TO IMPL IT DIRECTLY.. MAYBE DOSBOX-X GETS CONFUSED?
// AT ANY RATE THOUGH.. WE CAN JUST JUMP TO THE INSTRUCTION WE WANT AND LET IT EXECUTE!
#define RETURN_FAR() return HOOK_JUMP(0xb48, 0x01)  /* a far return is at this location in the binary */
#define RETURN_FAR_N(n) ({ \
  REMOVE_ARGS_FAR(n); \
  RETURN_FAR(); \
})

// XXX BROKEN: FIXME
/* #define HOOK_RET_FAR() ({ hook_result_t res = {HOOK_RESULT_TYPE_RET_FAR, -1, -1}; res; }) */


#define HOOK_RET_NEAR() ({ hook_result_t res = {HOOK_RESULT_TYPE_RET_NEAR, -1, -1}; res; })
#define RETURN_NEAR() return HOOK_RET_NEAR()

void hooks_init(void);
void hooklib_dump_cpu(hooklib_machine_registers_t *cpu);
void callstack_dump(void);
