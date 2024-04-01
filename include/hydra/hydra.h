#pragma once
#include <stdint.h>

#include "../../src/typedefs.h"
#include "../../src/addr.h"
#include "../../src/conf.h"
#include "../../src/hooklib.h"
#include "../../src/hooks.h"
#include "../../src/machine.h"
#include "../../src/functions.h"
#include "../../src/callstubs.h"

u8 * hydra_datasection_baseptr(void);
void hydra_callstack_dump(void);
