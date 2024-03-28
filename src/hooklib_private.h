#pragma once
#include <pthread.h>
#include "../dosbox-x/include/export/dosbox-x/hooklib.h"
#include "header.h"
#include "hooks.h"
#include "segoff.h"
#include "callstack.h"
#include "exec.h"
#include "machine.h"

// FIXME: REMOVE THIS HARDCODING
#define CODE_START_SEG ((u16)0x823)

#define ENABLE_DEBUG_CALLSTACK 0
#define MAX_HOOKS 2048
