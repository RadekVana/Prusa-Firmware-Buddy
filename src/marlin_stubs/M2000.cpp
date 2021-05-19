#include "../../lib/Marlin/Marlin/src/inc/MarlinConfig.h"
#include "../../lib/Marlin/Marlin/src/gcode/gcode.h"

#include "M2000.hpp"

static int val = 0;

int GetM2000() {
    return val;
}

void PrusaGcodeSuite::M2000() {
    if (parser.seen('X')) {
        val = parser.value_int();
    };
}
