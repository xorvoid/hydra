import sys

IMPL_STRING = '''\
/******************************************************************************/
/* Generate Callstubs */
/******************************************************************************/

FUNCTION_DEFINITIONS(DEFINE_CALLSTUB)

/******************************************************************************/
/* Generate Function Name Lookups */
/******************************************************************************/

typedef struct funcdef funcdef_t;
struct funcdef
{
  const char *name;
  segoff_t addr;
};

static inline const funcdef_t * function_defs(size_t *_nelts)
{
  static funcdef_t function_defs[] = {
#define DEFINE_FUNCDEF(name, _2, _3, seg, off, _6) {#name, {seg, off}},
    FUNCTION_DEFINITIONS(DEFINE_FUNCDEF)
#undef DEFINE_FUNCDEF
  };

  *_nelts = ARRAY_SIZE(function_defs);
  return function_defs;
}

static inline const funcdef_t * function_find(const char *name)
{
  size_t n = 0;
  const funcdef_t *defs = function_defs(&n);
  for (size_t i = 0; i < n; i++) {
    if (0 == strcmp(name, defs[i].name)) {
      return &defs[i];
    }
  }
  return NULL;
}

static inline const char *function_name(segoff_t s)
{
  size_t n_defs;
  const funcdef_t * defs = function_defs(&n_defs);

 u32 addr = segoff_abs(s);
  for (size_t i = 0; i < n_defs; i++) {
    const funcdef_t *f = &defs[i];
    if (addr == segoff_abs(f->addr)) {
      return f->name;
    }
  }
  return NULL;
}

static inline bool function_addr(const char *name, segoff_t *_out)
{
  size_t n_defs;
  const funcdef_t * defs = function_defs(&n_defs);

  for (size_t i = 0; i < n_defs; i++) {
    const funcdef_t *f = &defs[i];
    if (0 == strcmp(name, f->name)) {
      if (_out) *_out = f->addr;
      return true;
    }
  }
  return false;
}
'''

def gen_hdr(functions, out=None):
    f = sys.stdout if not out else out
    def emit(s): print(s, file=f)

    ## TODO: We no longer need the C X-MACRO style metaprogramming
    ## We should remove it and just generate everything from python directly
    ## We kept it for now to make it easy to get python code gen up and running

    emit('#pragma once')
    emit('#include "hydra/hydra.h"')
    emit('')
    emit('#define FUNCTION_DEFINITIONS(_)\\')
    emit('  /**********************************************************/\\')
    emit('  /* NAME                   RET   NARGS SEG OFF FLAGS */\\')
    emit('  /**********************************************************/\\')
    emit('  /* EXTRA NAMES: DON\'T GEN STUBS HERE: "IGNORE" */\\')

    for func in functions:
        if func.flags == 'INDIRECT_CALL_LOCATION': continue ## a call location, not a function location.. skip
        args = "IGNORE" if func.args < 0 else str(func.args)
        seg = f'0x{func.start_addr.seg:04x}'
        off = f'0x{func.start_addr.off:04x}'
        flags = str(func.flags)
        emit(f'  _( {func.name+",":30} {func.ret+",":8} {args+",":10} {seg+",":7} {off+",":7} {flags:15} )\\')

    emit('')
    emit(IMPL_STRING)
    emit('#undef FUNCTION_DEFINITIONS')
