# Hydra

Hydra is a runtime for reverse-engineering x86-16 MS-DOS binaries.
It is designed to support hybrid computation where some functions have been decompiled to ordinary C code and others remain in x86-16 machine-code.

## Goal

The overall goal of the Hydra runtime is to provide a platform to integrate decompiled code back into a running x86-16 MS-DOS binary.
A more traditional approach is to recompile to the origoinal target and link into the original binary. This approach is prohibitive for
a couple reasons:

1. The address-space on x86-16 is already highly constrained to 640KB and applications of the time already optimized extensively to utilize that limited platform. They
used several clever techniques such as overlays, calls to HIMEM, etc. Carving more out of this already constrained address space or trying to fit recompilations within the
original function's byte-rage is quite prohibitive.

2. Resurecting and using an ancient code-rotting compiler is also a challenging task for little benefit.

Instead, we built Hydra to allow us to compile decompiled functions to ordinary Mac M1 Aarch64 machine code, and to allow a hybrid computation model
that calls back-and-forth between the two different machines.

## X86-16 Emulation and Hooks

Hydra wraps the dosbox-x emulation to execute binaries. Dosbox-x has been forked and patched to capture machine-state and
provide hooks for hydra to interrupt its execution at any instruction address.

# Hydra function hooks

The main mechanism provided is a function hook such as follows:

```
HYDRA_FUNC(H_my_function)
{
  FRAME_ENTER(2);

  u16 arg = ARG_16(0x6);

  u16 result = F_some_other_function(m, arg);
  if (result > 1) {
    AX = 4;
  } else {
    AX = 5;
  }

  FRAME_LEAVE();
  RETURN_FAR();
}

void hook_init()
{
  HDYRA_REGISTER_ADDR(H_my_function, 0x0399, 0x0123);
}
```

When the x86-16 emulator reaches address `0399:0123`, hydra will interrupt the execution and call the `H_my_function`
routine above (running on Aarch64). This function can do pretty much anything to the `x86-16` machine state: modify
registers, write memory, call other x86-16 functions, return to arbitrary addresses, trigger an interrupt, read/write
to an I/O port, etc etc etc. The call to `F_some_other_function` is an example of calling an arbitrary function. This
function may be x86-16 machine code or may again be a hooked hydra function. When the function reaches `RETURN_FAR()`,
the Hydra Runtime will return back into the emulator using a `retf` equivalent return.

## Annotations system

Hydra also provides an extensive annotations metadata system with supports defining:

- Function names
- Global variables
- Jump Tables in the text section
- Callstack configuration data
- (and eventually) struct definitions

For example, one can access global variables that map to the same memory as the x86-16:

```
HYDRA_FUNC(H_my_function_2)
{
  FRAME_ENTER(0);

  G_some_global = 42;

  FRAME_LEAVE();
  RETURN_FAR();
}
```

## Other mechanisms provided

Hydra provides many other helpful features:
  - Callstack tracking and backtraces
  - Macros support for common 8086 operations: Registers, Flags, PUSH/POP, REP_MOVS, STI, INB, OUTB, CALL_FAR_INDIRECT, etc
  - ... and more ...

In addition, [dis86](https://github.com/xorvoid/dis86) is designed to generate code that compiles and runs correctly with hydra.
