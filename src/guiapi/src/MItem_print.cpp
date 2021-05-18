#include "MItem_print.hpp"
#include "marlin_client.h"
#include "menu_vars.h"
#include "eeprom.h"
#include "menu_spin_config.hpp"
/*****************************************************************************/
//MI_NOZZLE
MI_NOZZLE::MI_NOZZLE()
    : WI_SPIN_U16_t(uint16_t(marlin_vars()->target_nozzle),
        SpinCnf::nozzle, _(label), 0, is_enabled_t::yes, is_hidden_t::no) {}
void MI_NOZZLE::OnClick() {
    marlin_set_target_nozzle(GetVal());
    marlin_set_display_nozzle(GetVal());
}

/*****************************************************************************/
//MI_HEATBED
MI_HEATBED::MI_HEATBED()
    : WI_SPIN_U08_t(uint8_t(marlin_vars()->target_bed),
        SpinCnf::bed, _(label), 0, is_enabled_t::yes, is_hidden_t::no) {}
void MI_HEATBED::OnClick() {
    marlin_set_target_bed(GetVal());
}

/*****************************************************************************/
//MI_PRINTFAN
MI_PRINTFAN::MI_PRINTFAN()
    : WI_SPIN_U08_t(uint8_t(marlin_vars()->fan_speed),
        SpinCnf::printfan, _(label), 0, is_enabled_t::yes, is_hidden_t::no) {}
void MI_PRINTFAN::OnClick() {
    marlin_set_fan_speed(GetVal());
}

/*****************************************************************************/
//MI_SPEED
MI_SPEED::MI_SPEED()
    : WI_SPIN_U16_t(uint16_t(marlin_vars()->print_speed),
        SpinCnf::feedrate, _(label), 0, is_enabled_t::yes, is_hidden_t::no) {}
void MI_SPEED::OnClick() {
    marlin_set_print_speed(GetVal());
}

/*****************************************************************************/
//MI_FLOWFACT
MI_FLOWFACT::MI_FLOWFACT()
    : WI_SPIN_U16_t(uint16_t(marlin_vars()->flow_factor),
        SpinCnf::flowfact, _(label), 0, is_enabled_t::yes, is_hidden_t::no) {}
void MI_FLOWFACT::OnClick() {
    marlin_set_flow_factor(GetVal());
}

/*****************************************************************************/
//MI_M876_XVAL
const SpinConfig_I32_t &Get_M876_range();
int32_t Get_M876_XVal();
void Set_M876_XVal(int32_t val);

MI_M876_XVAL::MI_M876_XVAL()
    : WI_SPIN_I32_t(Get_M876_XVal(), Get_M876_range(),
        _(label), 0, is_enabled_t::yes, is_hidden_t::no) {}
void MI_M876_XVAL::OnClick() {
    Set_M876_XVal(GetVal());
}
