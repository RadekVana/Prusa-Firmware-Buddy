/**
 * @file filament_sensor.cpp
 * @author Radek Vana
 * @date 2019-12-16
 */

#include "filament_sensor.hpp"
#include "print_processor.hpp"
#include "fsensor_eeprom.hpp"
#include "rtos_api.hpp"

FSensor::FSensor()
    : status({ 0, send_M600_t::on_edge, 0 })
    , old_fs_state(false)
    , run_first(true)
    , current_detect_filament_insert(false)
    , state(fsensor_t::NotInitialized)
    , last_state(fsensor_t::NotInitialized) {
}

/*---------------------------------------------------------------------------*/
//debug functions
bool FSensor::WasM600_send() {
    return status.M600_sent != 0;
}

char FSensor::GetM600_send_on() {
    switch (status.send_M600_on) {
    case send_M600_t::on_edge:
        return 'e';
    case send_M600_t::on_level:
        return 'l';
    case send_M600_t::never:
        return 'n';
    }
    return 'x';
}

//simple filter
//without filter fs_meas_cycle1 could set FS_NO_SENSOR (in case filament just runout)
void FSensor::set_state(fsensor_t st) {
    CriticalSection C;
    if (last_state == st)
        state = st;
    last_state = st;
}

/*---------------------------------------------------------------------------*/
//global thread safe functions
fsensor_t FSensor::Get() {
    return state;
}

//value can change during read, but it is not a problem
bool FSensor::DidRunOut() {
    return state == fsensor_t::NoFilament;
}

void FSensor::M600_on_edge() {
    CriticalSection C;
    status.send_M600_on = send_M600_t::on_edge;
}

void FSensor::M600_on_level() {
    CriticalSection C;
    status.send_M600_on = send_M600_t::on_level;
}

void FSensor::M600_never() {
    CriticalSection C;
    status.send_M600_on = send_M600_t::never;
}

/*---------------------------------------------------------------------------*/
//global thread safe functions
//but cannot be called from interrupt
void FSensor::Enable() {
    CriticalSection C;
    enable();
    FSensorEEPROM::Set();
}

void FSensor::Disable() {
    CriticalSection C;
    disable();
    FSensorEEPROM::Clr();
}

FSensor::send_M600_t FSensor::getM600_send_on_and_disable() {
    CriticalSection C;
    send_M600_t ret = status.send_M600_on;
    status.send_M600_on = send_M600_t::never;
    return ret;
}
void FSensor::restore_send_M600_on(FSensor::send_M600_t send_M600_on) {
    CriticalSection C;
    //cannot call init(); - it could cause stacking in uninitialized state
    status.send_M600_on = send_M600_on;
}

fsensor_t FSensor::WaitInitialized() {
    fsensor_t ret = FSensor::Get();
    while (ret == fsensor_t::NotInitialized) {
        Rtos::Delay(0); // switch to other threads
        ret = FSensor::Get();
    }
    return ret;
}

void FSensor::ClrSent() {
    CriticalSection C;
    status.M600_sent = 0;
}

/*---------------------------------------------------------------------------*/
//global not thread safe functions
void FSensor::init() {
    bool enabled = FSensorEEPROM::Get();

    if (enabled)
        enable();
    else
        disable();
}

void FSensor::InitOnEdge() {
    init();
    FSensor::M600_on_edge();
}
void FSensor::InitOnLevel() {
    init();
    FSensor::M600_on_level();
}
void FSensor::InitNever() {
    init();
    FSensor::M600_never();
}

/*---------------------------------------------------------------------------*/
//methods called only in fs_cycle
void FSensor::injectM600() {
    if (status.M600_sent == 0 && (PrintProcessor::IsPrinting() && !PrintProcessor::IsM600injectionBlocked())) {
        PrintProcessor::InjectGcode("M600"); //change filament
        status.M600_sent = 1;
    }
}

//M600_on_edge == inject after state was changed from HasFilament to NoFilament
//M600_on_level == inject on NoFilament
//M600_never == do not inject
void FSensor::evaluate_M600_conditions() {
    bool had_filament = state == fsensor_t::HasFilament;

    if (state == fsensor_t::NoFilament) {
        switch (status.send_M600_on) {
        case send_M600_t::on_edge:
            if (!had_filament)
                break;
            // no break if had_filament == true
        case send_M600_t::on_level:
            injectM600();
            break;
        case send_M600_t::never:
        default:
            break;
        }
    }
}

void FSensor::autoload_loop() {
    if (PrintProcessor::IsAutoloadEnabled()) {
        if (FSensor::Get() == fsensor_t::HasFilament) {
            current_detect_filament_insert = true;
        } else if (FSensor::Get() == fsensor_t::NoFilament) {
            current_detect_filament_insert = false;
            run_first = false;
        }

        if (current_detect_filament_insert != old_fs_state && current_detect_filament_insert == true && run_first == false) {
            if (!PrintProcessor::IsPrinting()) {
                PrintProcessor::InjectGcode("M1400 S1");
            }
        }

        old_fs_state = current_detect_filament_insert;
    }
}

//delay between calls must be 1us or longer
void FSensor::Cycle() {
    //sensor is disabled (only init can enable it)
    if (state == fsensor_t::Disabled)
        return;

    PrintProcessor::Update();

    //sensor is enabled
    cycle();

    autoload_loop();
}
