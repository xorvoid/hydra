import sys

def gen_hdr(datasection, out=None):
    f = sys.stdout if not out else out
    def emit(s): print(s, file=f)

    emit('#pragma once')
    emit('#include "hydra/hydra.h"')
    emit('#include "header.h"')
    emit('#include "structures.h"')
    emit('')

    for var in datasection:
        cast = f'({var.typ.basetype}*)'
        start = cast if var.typ.is_array else '*'+cast
        end = ''
        if var.typ.is_array:
            end = f' /* array: {var.typ} */'
        emit(f'#define {var.name:30} ({start:15} (hydra_datasection_baseptr() + 0x{var.off:04x})){end}')
