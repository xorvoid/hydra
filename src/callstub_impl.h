#pragma once

#define DONT_POP_ARGS 1
#define NEAR 2

#define DEFINE_CALLSTUB(name, ret, nargs, seg, off, flags) \
  DEFINE_CALLSTUB_ ## nargs(name, ret, seg, off, flags)

#define DEFINE_CALLSTUB_IGNORE(name, ret, seg, off, flags)

#define DEFINE_CALLSTUB_0(name, ret, seg, off, flags) static ret name(hooklib_machine_t *m) { return (ret)_impl_callstub_0(m, seg, off, flags); }

#define DEFINE_CALLSTUB_1(name, ret, seg, off, flags) static ret name(hooklib_machine_t *m, u16 arg1) { return (ret)_impl_callstub_1(m, seg, off, flags, arg1); }

#define DEFINE_CALLSTUB_2(name, ret, seg, off, flags) static ret name(hooklib_machine_t *m, u16 arg1, u16 arg2) { return (ret)_impl_callstub_2(m, seg, off, flags, arg1, arg2); }

#define DEFINE_CALLSTUB_3(name, ret, seg, off, flags) static ret name(hooklib_machine_t *m, u16 arg1, u16 arg2, u16 arg3) { return (ret)_impl_callstub_3(m, seg, off, flags, arg1, arg2, arg3); }

#define DEFINE_CALLSTUB_4(name, ret, seg, off, flags) static ret name(hooklib_machine_t *m, u16 arg1, u16 arg2, u16 arg3, u16 arg4) { return (ret)_impl_callstub_4(m, seg, off, flags, arg1, arg2, arg3, arg4); }

#define DEFINE_CALLSTUB_5(name, ret, seg, off, flags) static ret name(hooklib_machine_t *m, u16 arg1, u16 arg2, u16 arg3, u16 arg4, u16 arg5) { return (ret)_impl_callstub_5(m, seg, off, flags, arg1, arg2, arg3, arg4, arg5); }

#define DEFINE_CALLSTUB_6(name, ret, seg, off, flags) static ret name(hooklib_machine_t *m, u16 arg1, u16 arg2, u16 arg3, u16 arg4, u16 arg5, u16 arg6) { return (ret)_impl_callstub_6(m, seg, off, flags, arg1, arg2, arg3, arg4, arg5, arg6); }


static void _impl_call(hooklib_machine_t *m, u16 seg, u16 off, int flags)
{
  if (flags & NEAR) CALL_NEAR(16*(u32)seg + off);
  else CALL_FAR(seg, off);
}

static u32 _impl_callstub_0(hooklib_machine_t *m, u16 seg, u16 off, int flags)
{
  _impl_call(m, seg, off, flags);
  return (u32)DX << 16 | AX;
}

static u32 _impl_callstub_1(hooklib_machine_t *m, u16 seg, u16 off, int flags, u16 arg1)
{
  PUSH(arg1);
  _impl_call(m, seg, off, flags);
  if (!(flags & DONT_POP_ARGS)) SP += 0x2;
  return (u32)DX << 16 | AX;
}

static u32 _impl_callstub_2(hooklib_machine_t *m, u16 seg, u16 off, int flags, u16 arg1, u16 arg2)
{
  PUSH(arg2);
  PUSH(arg1);
  _impl_call(m, seg, off, flags);
  if (!(flags & DONT_POP_ARGS)) SP += 0x4;
  return (u32)DX << 16 | AX;
}

static u32 _impl_callstub_3(hooklib_machine_t *m, u16 seg, u16 off, int flags, u16 arg1, u16 arg2, u16 arg3)
{
  PUSH(arg3);
  PUSH(arg2);
  PUSH(arg1);
  _impl_call(m, seg, off, flags);
  if (!(flags & DONT_POP_ARGS)) SP += 0x6;
  return (u32)DX << 16 | AX;
}

static u32 _impl_callstub_4(hooklib_machine_t *m, u16 seg, u16 off, int flags, u16 arg1, u16 arg2, u16 arg3, u16 arg4)
{
  PUSH(arg4);
  PUSH(arg3);
  PUSH(arg2);
  PUSH(arg1);
  _impl_call(m, seg, off, flags);
  if (!(flags & DONT_POP_ARGS)) SP += 0x8;
  return (u32)DX << 16 | AX;
}

static u32 _impl_callstub_5(hooklib_machine_t *m, u16 seg, u16 off, int flags, u16 arg1, u16 arg2, u16 arg3, u16 arg4, u16 arg5)
{
  PUSH(arg5);
  PUSH(arg4);
  PUSH(arg3);
  PUSH(arg2);
  PUSH(arg1);
  _impl_call(m, seg, off, flags);
  if (!(flags & DONT_POP_ARGS)) SP += 0xa;
  return (u32)DX << 16 | AX;
}

static u32 _impl_callstub_6(hooklib_machine_t *m, u16 seg, u16 off, int flags, u16 arg1, u16 arg2, u16 arg3, u16 arg4, u16 arg5, u16 arg6)
{
  PUSH(arg6);
  PUSH(arg5);
  PUSH(arg4);
  PUSH(arg3);
  PUSH(arg2);
  PUSH(arg1);
  _impl_call(m, seg, off, flags);
  if (!(flags & DONT_POP_ARGS)) SP += 0xc;
  return (u32)DX << 16 | AX;
}
