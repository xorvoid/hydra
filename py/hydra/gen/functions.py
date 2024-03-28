import sys

def gen_hdr(functions, out=None):
    f = sys.stdout if not out else out
    def emit(s): print(s, file=f)

    ## TODO: We no longer need the C X-MACRO style metaprogramming
    ## We should remove it and just generate everything from python directly
    ## We kept it for now to make it easy to get python code gen up and running

    emit('#pragma once')
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
