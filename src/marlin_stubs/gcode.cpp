#include "../../lib/Marlin/Marlin/src/inc/MarlinConfig.h"
#include "../../lib/Marlin/Marlin/src/gcode/gcode.h"

bool GcodeSuite::process_parsed_command_custom(bool no_ok) {
    switch (parser.command_letter) {
    case 'G':
        switch (parser.codenum) {
        case 162:

            return true;
        }
        return false;
    default:
        return false;
    }
}
