
typedef struct hydra_hook   hydra_hook_t;
typedef struct hydra_result hydra_result_t;

enum {
  HYDRA_RESULT_TYPE_RESUME,
  HYDRA_RESULT_TYPE_JUMP,
  HYDRA_RESULT_TYPE_JUMP_NEAR,
  HYDRA_RESULT_TYPE_CALL,
  HYDRA_RESULT_TYPE_CALL_NEAR,
  HYDRA_RESULT_TYPE_RET_NEAR,
  /* HYDRA_RESULT_TYPE_RET_FAR, */
};

struct hydra_result
{
  int type;
  uint16_t new_cs;
  uint16_t new_ip;
};

enum {
  HYDRA_HOOK_FLAGS_OVERLAY = 1<<0,
};

struct hydra_hook
{
  hydra_result_t (*func)(hooklib_machine_t *);
  uint16_t hook_cs, hook_ip;
  int flags;
};

void hydra_hook_register(hydra_hook_t entry);
hydra_hook_t * hydra_hook_find(segoff_t addr);

// OLD
#define HOOK_REGISTER(func, seg, off, flags) do {  \
  hydra_hook_t ent = {(func), (seg), (off), (flags)}; \
  hydra_hook_register(ent); \
} while(0)

// NEW
#define HOOK_REG(name, flags) do {  \
    const char *f_name = "F_" #name; \
    hydra_result_t (*h_func)(hooklib_machine_t *) = H_ ## name; \
    const hydra_function_def_t *def = hydra_function_find(f_name);                      \
    if (!def) FAIL("Cannot find function '%s' to register", f_name); \
    hydra_hook_t ent = {h_func, def->addr.seg, def->addr.off, (flags)}; \
  hydra_hook_register(ent);                      \
} while(0)

hydra_result_t H_DEAD(hooklib_machine_t *m);
#define HOOK_DEAD(func, seg, off, flags) HOOK_REGISTER(H_DEAD, seg, off, flags)

#define HOOK_FUNC(name) hydra_result_t name(hooklib_machine_t *m)

#define HOOK_RESUME() ({ hydra_result_t res = {HYDRA_RESULT_TYPE_RESUME, -1, -1}; res; })
#define HOOK_JUMP(seg, off) ({ hydra_result_t res = {HYDRA_RESULT_TYPE_JUMP, seg, off}; res; })
#define HOOK_JUMP_NEAR(off) ({ hydra_result_t res = {HYDRA_RESULT_TYPE_JUMP_NEAR, 0, off}; res; })

// JUMP TO WHERE A NEAR OR FAR RET IS (LOL)
// FOR SOME REASON WE CAN SEEM TO IMPL IT DIRECTLY.. MAYBE DOSBOX-X GETS CONFUSED?
// AT ANY RATE THOUGH.. WE CAN JUST JUMP TO THE INSTRUCTION WE WANT AND LET IT EXECUTE!
#define RETURN_FAR() return HOOK_JUMP(0xb48, 0x01)  /* a far return is at this location in the binary */
#define RETURN_FAR_N(n) ({ \
  REMOVE_ARGS_FAR(n); \
  RETURN_FAR(); \
})

// XXX BROKEN: FIXME
/* #define HOOK_RET_FAR() ({ result_t res = {RESULT_TYPE_RET_FAR, -1, -1}; res; }) */


#define HOOK_RET_NEAR() ({ hydra_result_t res = {HYDRA_RESULT_TYPE_RET_NEAR, -1, -1}; res; })
#define RETURN_NEAR() return HOOK_RET_NEAR()

// TODO: MOVE THIS SOMEWHERE BETTER?
void hydra_cpu_dump(hooklib_machine_registers_t *cpu);
