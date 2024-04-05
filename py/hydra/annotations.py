def basetype_size_in_bytes(typename):
    d = {
        'u8':  1,
        'u16': 2,
        'u32': 3,
    }
    return d.get(typename, None)

class Type:
    def __init__(self, basetype, is_array, array_len):
        self.basetype = basetype
        self.is_array = is_array
        self.array_len = array_len

    @staticmethod
    def from_str(s):
        parts = s.split('[')
        if len(parts) > 2:
            raise Expception(f'Invalid type: "{s}"')

        typ = Type(parts[0], False, None)
        if len(parts) > 1:
            typ.is_array = True
            left = parts[1]
            if len(left) == 0 or left[-1] != ']':
                raise Exception('Expected closing brace in array type')
            size_str = left[:-1]
            if len(size_str) > 0:
                typ.array_len = int(size_str)
            else:
                typ.array_len = None
        return typ

    def as_basetype(self):
        return Type(self.basetype, False, None)

    def size_in_bytes(self):
        basesz = basetype_size_in_bytes(self.basetype)
        if not self.is_array: return basesz

        if basesz is None: return None
        if self.array_len is None: return None

        return basesz * self.array_len

    def __str__(self):
        s = self.basetype
        if self.is_array:
            sz = '' if self.array_len is None else str(self.array_len)
            s += f'[{sz}]'
        return s

class Off:
    def __init__(self, off):
        self.off = int(off, 16)
        assert 0 <= self.off and self.off < (1<<16)

    def __str__(self):
        return f'0x{self.off:04x}'

class Addr:
    def __init__(self, addr):
        parts = addr.split(':')
        if len(parts) != 2: raise Exception(f'Invalid address: "{addr}"')
        self.seg = int(parts[0], 16)
        self.off = int(parts[1], 16)

    def abs(self):
        return self.seg * 16 + self.off

    def __str__(self):
        return f'{self.seg:04x}:{self.off:04x}'

### FIXME RENAME
UNKNOWN=-1

class Function:
    def __init__(self, name, ret, args, start_addr, end_addr, flags=0, overlay=None):
        self.name = name
        self.ret = ret
        self.args = args
        self.start_addr = Addr(start_addr)
        self.end_addr = Addr(end_addr) if end_addr else None
        self.overlay_num = overlay[0] if overlay else None
        self.overlay_start = Off(overlay[1]) if overlay else None
        self.overlay_end = Off(overlay[2]) if overlay else None
        self.flags = flags

class Global:
    def __init__(self, name, typ, off, flags=''):
        self.name = name
        self.typ = Type.from_str(typ)
        self.off = off
        self.flags = flags

def validate_data_section(ds):
    mem = [0] * (1<<16)
    for g in ds:
        if g.flags == 'SKIP_VALIDATE':
            continue
        off = g.off
        sz = g.typ.size_in_bytes()
        if sz is None:
            print('WARN: Cannot determine size for %s' % g.name)
            continue
        for i in range(off, off+sz):
            if mem[i] != 0:
                raise Exception('Overlap detect in %s: [0x%04x, 0x%04x]' % (g.name, off, off+sz))
            mem[i] = 1

class TextData:
    def __init__(self, name, typ, start_addr, end_addr):
        self.name = name
        self.typ = Type.from_str(typ)
        self.start_addr = Addr(start_addr)
        self.end_addr = Addr(end_addr)

        ## infer array size
        nbytes = self.end_addr.abs() - self.start_addr.abs()
        if nbytes < 0: raise Exception(f"Negatively sized text-section region: {name}")
        if not self.typ.is_array: raise Exception(f"Expected array for text-section region: {name}")
        eltsz = self.typ.as_basetype().size_in_bytes()
        if nbytes % eltsz != 0: raise Exception(f"Expected text-section region to be a multiple of {eltsz}: {name}")
        array_len = nbytes // eltsz

        ## use it or verify
        if self.typ.array_len is not None:
            if self.typ.array_len != array_len: raise Exception(f"Misconfiguration: config specified {self.typ.array_len} elements, but region contains {array_len}: {name}")
        self.typ.array_len = array_len

CALLSTACK_CONF_VALID_TYPES = { 'HANDLER', 'IGNORE_ADDR', 'JUMPRET', }

class CallstackConf:
    def __init__(self, name, typ, addr):
        if typ not in CALLSTACK_CONF_VALID_TYPES:
            raise Exception(f'Not a valid callstack conf type: {typ}')
        self.name = name
        self.typ  = typ
        self.addr = Addr(addr)
