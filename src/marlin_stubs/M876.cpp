/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "../../lib/Marlin/Marlin/src/inc/MarlinConfig.h"

#if ENABLED(HOST_PROMPT_SUPPORT) && DISABLED(EMERGENCY_PARSER)

    #include "../../lib/Marlin/Marlin/src/feature/host_actions.h"
    #include "safety_timer_stubbed.hpp"
    #include "../../lib/Marlin/Marlin/src/gcode/gcode.h"
    #include "../../lib/Marlin/Marlin/src/Marlin.h"
    #include "marlin_server.hpp"
    #include "client_fsm_types.h"

    #include "menu_spin_config_type.hpp"
    #include <algorithm>

const SpinConfig_I32_t range = { { 0, 1000, 1 } };
int32_t X_val = range.Min();
const SpinConfig_I32_t &Get_M876_range() { return range; }
int32_t Get_M876_XVal() { return X_val; }
void Set_M876_XVal(int32_t val) { X_val = std::clamp<int32_t>(val, range.Min(), range.Max()); }

/**
 * M876: Handle Prompt Response
 *
 * E display error popup other parameters are ignored
 * E value must be smaller than WarningType::_count
 */
void GcodeSuite::M876() {

    if (parser.seenval('X')) {
        Set_M876_XVal(parser.value_int());
    }

    //mainly for debug
    if (parser.seenval('E')) {
        uint32_t val = parser.value_int();
        if (val > uint32_t(WarningType::_last))
            return;
        set_warning(WarningType(val));
    } else {

        if (parser.seenval('P')) {
            if (parser.value_int()) {
                fsm_create(ClientFSM::Serial_printing);
            } else {
                fsm_destroy(ClientFSM::Serial_printing);
                SafetyTimer::Instance().ReInit(); // in miliseconds
            }
        }
        if (parser.seenval('S'))
            host_response_handler((uint8_t)parser.value_int());
    }
}

#endif // HOST_PROMPT_SUPPORT && !EMERGENCY_PARSER
