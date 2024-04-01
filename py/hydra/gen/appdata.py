import sys

def gen_hdr(data, out=None):
    f = sys.stdout if not out else out
    def emit(s): print(s, file=f)

    functions = data['functions']
    datasection = data['data_section']

    emit('#pragma once')
    emit('#include "hydra/hydra.h"')
    emit('#if __has_include ("hydra_user_defs.h")')
    emit('  #include "hydra_user_defs.h"')
    emit('#endif')
    emit('')

    ## TODO: We no longer need the C X-MACRO style metaprogramming
    ## We should remove it and just generate everything from python directly
    ## We kept it for now to make it easy to get python code gen up and running

    emit('/**************************************************************************************************************/')
    emit('/* Functions */')
    emit('/**************************************************************************************************************/')
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

    emit('/**************************************************************************************************************/')
    emit('/* Generate Callstubs */')
    emit('/**************************************************************************************************************/')
    emit('')
    emit('FUNCTION_DEFINITIONS(DEFINE_CALLSTUB)')
    emit('')

    emit('/**************************************************************************************************************/')
    emit('/* Data Section Globals */')
    emit('/**************************************************************************************************************/')
    emit('')

    for var in datasection:
        cast = f'({var.typ.basetype}*)'
        start = cast if var.typ.is_array else '*'+cast
        end = ''
        if var.typ.is_array:
            end = f' /* array: {var.typ} */'
        emit(f'#define {var.name:30} ({start:15} (hydra_datasection_baseptr() + 0x{var.off:04x})){end}')

def gen_src(data, out=None):
    f = sys.stdout if not out else out
    def emit(s): print(s, file=f)

    emit('#include "hydra_user_appdata.h"')
    emit('')
    emit('/**************************************************************************************************************/')
    emit('/* Generate Function Metdata */')
    emit('/**************************************************************************************************************/')
    emit('')
    emit('const function_metadata_t * hydra_user_functions(void)')
    emit('{')
    emit('  static function_def_t defs[] = {')
    emit('#define DEFINE_FUNCDEF(name, _2, _3, seg, off, _6) {#name, {seg, off}},')
    emit('    FUNCTION_DEFINITIONS(DEFINE_FUNCDEF)')
    emit('#undef DEFINE_FUNCDEF')
    emit('  };')
    emit('')
    emit('  static function_metadata_t md[1];')
    emit('  md->n_defs = sizeof(defs)/sizeof(defs[0]);')
    emit('  md->defs = defs;')
    emit('')
    emit('  return md;')
    emit('}')
    emit('')
