import sys

def gen_hdr(data, out=None):
    f = sys.stdout if not out else out
    def emit(s): print(s, file=f)

    functions = data['functions']
    datasection = data['data_section']
    structures = data['structures']

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
    emit('#define HYDRA_FUNCTION_DEFINITIONS(_)\\')
    emit('  /**********************************************************/\\')
    emit('  /* NAME                   RET   NARGS SEG OFF FLAGS */\\')
    emit('  /**********************************************************/\\')
    emit('  /* EXTRA NAMES: DON\'T GEN STUBS HERE: "IGNORE" */\\')

    for func in functions:
        if func.flags == 'INDIRECT_CALL_LOCATION': continue ## a call location, not a function location.. skip
        args = "IGNORE" if func.args < 0 else str(func.args)
        if func.start_addr is None:
            continue
        seg = f'0x{func.start_addr.seg:04x}'
        off = f'0x{func.start_addr.off:04x}'
        flags = str(func.flags)
        emit(f'  _( {func.name+",":30} {func.ret+",":8} {args+",":10} {seg+",":7} {off+",":7} {flags:15} )\\')

    emit('')

    emit('/**************************************************************************************************************/')
    emit('/* Generate Callstubs */')
    emit('/**************************************************************************************************************/')
    emit('')
    emit('HYDRA_FUNCTION_DEFINITIONS(HYDRA_DEFINE_CALLSTUB)')
    emit('')

    emit('/**************************************************************************************************************/')
    emit('/* Define IS_OVERLAY flags */')
    emit('/**************************************************************************************************************/')
    for func in functions:
        emit('#define IS_OVERLAY_' + func.name + ' ' + ('1' if func.is_overlay else '0'))
    emit('')

    emit('/**************************************************************************************************************/')
    emit('/* Structures */')
    emit('/**************************************************************************************************************/')
    emit('')

    for struct in structures:
        emit(f'typedef struct {struct.struct_name()} {struct.name};')
        emit(f'struct __attribute__((packed)) {struct.struct_name()}')
        emit('{')
        for mbr in struct.members:
            mbr_entry = f'{mbr.typ.fmt_ctype_str(mbr.name)};'
            emit(f'  {mbr_entry:<40}  /* 0x{mbr.off:02x} */')
        emit('};')
        emit(f'static_assert(sizeof({struct.name}) == {struct.size}, "");');
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
    emit('const hydra_function_metadata_t * hydra_user_functions(void)')
    emit('{')
    emit('  static hydra_function_def_t defs[] = {')
    emit('#define DEFINE_FUNCDEF(name, _2, _3, seg, off, _6) {#name, {seg, off}},')
    emit('    HYDRA_FUNCTION_DEFINITIONS(DEFINE_FUNCDEF)')
    emit('#undef DEFINE_FUNCDEF')
    emit('  };')
    emit('')
    emit('  static hydra_function_metadata_t md[1];')
    emit('  md->n_defs = sizeof(defs)/sizeof(defs[0]);')
    emit('  md->defs = defs;')
    emit('')
    emit('  return md;')
    emit('}')
    emit('')
    emit('const hydra_callstack_metadata_t * hydra_user_callstack(void)')
    emit('{')
    emit('  static hydra_callstack_conf_t confs[] = {')
    for conf in data['callstack']:
        type_str = f'HYDRA_CALLSTACK_CONF_TYPE_{conf.typ},'
        name_str = f'"{conf.name}",'
        emit(f'    {{ {type_str:<40} {name_str:<25} {{ 0x{conf.addr.seg:04x}, 0x{conf.addr.off:04x} }} }},')
    emit('  };')
    emit('')
    emit('  static hydra_callstack_metadata_t md[1];')
    emit('  md->n_confs = sizeof(confs)/sizeof(confs[0]);')
    emit('  md->confs = confs;')
    emit('')
    emit('  return md;')
    emit('}')
    emit('')
