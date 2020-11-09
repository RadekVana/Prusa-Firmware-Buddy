/**
 * @file menu_spin_config_basic.cpp
 * @author Radek Vana
 * @brief
 * @version 0.1
 * @date 2020-11-04
 *
 * @copyright Copyright (c) 2020
 *
 * included by menu_spin_config.hpp
 * menu_spin_config_with_units.cpp must not be linked
 */
#include "menu_spin_config.hpp"

// SpinConfig_t == SpinConfig
const SpinConfig_U16_t SpinCnf::nozzle = SpinConfig_U16_t(MenuVars::nozzle_range);
const SpinConfig_U08_t SpinCnf::bed = SpinConfig_U08_t(MenuVars::bed_range);
const SpinConfig_U08_t SpinCnf::printfan = SpinConfig_U08_t(MenuVars::printfan_range);
const SpinConfig_U16_t SpinCnf::feedrate = SpinConfig_U16_t(MenuVars::feedrate_range);
const SpinConfig_U16_t SpinCnf::flowfact = SpinConfig_U16_t(MenuVars::flowfact_range);
const SpinConfig_I08_t SpinCnf::timezone_range = { { -12, 12, 1 } };
const SpinConfig_U08_t SpinCnf::volume_range = { { 0, 10, 1 } };
const std::array<SpinConfig_I16_t, MenuVars::AXIS_CNT> SpinCnf::axis_ranges = { axis_ranges };
