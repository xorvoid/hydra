#pragma once
#include <stdint.h>

#include "../../src/typedefs.h"
#include "../../src/segoff.h"
#include "../../src/hooklib.h"
#include "../../src/hooks.h"
#include "../../src/machine.h"
#include "../../src/functions.h"
#include "../../src/callstubs.h"

u8 * hydra_datasection_baseptr(void);
void hydra_datasection_baseptr_set(u8 *ptr);
void hydra_callstack_dump(void);
