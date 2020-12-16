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

/**
 * feature/pause.cpp - Pause feature support functions
 * This may be combined with related G-codes if features are consolidated.
 */
#include "../../lib/Marlin/Marlin/src/inc/MarlinConfigPre.h"

#include "../../lib/Marlin/Marlin/src/Marlin.h"
#include "../../lib/Marlin/Marlin/src/gcode/gcode.h"
#include "../../lib/Marlin/Marlin/src/module/motion.h"
#include "../../lib/Marlin/Marlin/src/module/planner.h"
#include "../../lib/Marlin/Marlin/src/module/stepper.h"
#include "../../lib/Marlin/Marlin/src/module/printcounter.h"
#include "../../lib/Marlin/Marlin/src/module/temperature.h"

#if ENABLED(FWRETRACT)
    #include "fwretract.h"
#endif

#include "../../lib/Marlin/Marlin/src/lcd/extensible_ui/ui_api.h"
#include "../../lib/Marlin/Marlin/src/core/language.h"
#include "../../lib/Marlin/Marlin/src/lcd/ultralcd.h"

#include "../../lib/Marlin/Marlin/src/libs/nozzle.h"
#include "../../lib/Marlin/Marlin/src/feature/pause.h"

#include "pause_stubbed.hpp"

#include "marlin_server.hpp"
#include "filament_sensor.hpp"
#include "filament.h"
#include "RAII.hpp"
#include <cmath>

// private:
//check unsupported features
//filament sensor is no longer part of marlin thus it must be disabled
//HAS_BUZZER must be disabled, because we handle it differently than marlin
// clang-format off
#if (!ENABLED(EXTENSIBLE_UI)) || \
    (!ENABLED(ADVANCED_PAUSE_FEATURE)) || \
    HAS_FILAMENT_SENSOR || \
    HAS_BUZZER || \
    HAS_LCD_MENU || \
    NUM_RUNOUT_SENSORS > 1 || \
    ENABLED(DUAL_X_CARRIAGE) || \
    (!ENABLED(PREVENT_COLD_EXTRUSION)) || \
    ENABLED(ADVANCED_PAUSE_CONTINUOUS_PURGE) || \
    BOTH(FILAMENT_UNLOAD_ALL_EXTRUDERS, MIXING_EXTRUDER) || \
    ENABLED(SDSUPPORT)
#error unsupported
#endif
// clang-format on

PauseMenuResponse pause_menu_response;

//cannot be class member (externed in marlin)
uint8_t did_pause_print = 0;
fil_change_settings_t fc_settings[EXTRUDERS];

//cannot be class member (externed in marlin and used by M240 and tool_change)
void do_pause_e_move(const float &length, const feedRate_t &fr_mm_s) {
    current_position.e += length / planner.e_factor[active_extruder];
    line_to_current_position(fr_mm_s);
    planner.synchronize();
}

/*****************************************************************************/
//Pause
Pause &pause = Pause::GetInstance();

Pause &Pause::GetInstance() {
    static Pause s;
    return s;
}

void Pause::SetUnloadLength(float len) {
    unload_length = -std::abs(len); // it is negative value
}

void Pause::SetSlowLoadLength(float len) {
    slow_load_length = std::abs(len);
}

void Pause::SetFastLoadLength(float len) {
    fast_load_length = std::abs(len);
}

void Pause::SetPurgeLength(float len) {
    purge_length = std::max(std::abs(len), (float)minimal_purge);
}

float Pause::GetDefaultLoadLength() const {
    return fc_settings[active_extruder].load_length;
}

float Pause::GetDefaultUnloadLength() const {
    return fc_settings[active_extruder].unload_length;
}

bool Pause::is_target_temperature_safe() {
    if (!DEBUGGING(DRYRUN) && thermalManager.targetTooColdToExtrude(active_extruder)) {
        SERIAL_ECHO_MSG(MSG_ERR_HOTEND_TOO_COLD);
        return false;
    } else {
        return true;
    }
}

bool Pause::ensure_safe_temperature_notify_progress(PhasesLoadUnload phase, uint8_t progress_min, uint8_t progress_max) {

    if (!is_target_temperature_safe()) {
        return false;
    }

    if (Temperature::degHotend(active_extruder) + heating_phase_min_hotend_diff > Temperature::degTargetHotend(active_extruder)) { //do not disturb user with heating dialog
        return true;
    }

    Notifier_TEMP_NOZ N(ClientFSM::Load_unload, GetPhaseIndex(phase),
        Temperature::degHotend(active_extruder), Temperature::degTargetHotend(active_extruder), progress_min, progress_max);

    return thermalManager.wait_for_hotend(active_extruder);
}

void Pause::do_e_move_notify_progress(const float &length, const feedRate_t &fr_mm_s, PhasesLoadUnload phase, uint8_t progress_min, uint8_t progress_max) {
    //Not sure if folowing code would not be better
    //const float actual_e = planner.get_axis_position_mm(E_AXIS);
    //Notifier_POS_E N(ClientFSM::Load_unload, GetPhaseIndex(phase), actual_e, actual_e + length, progress_min,progress_max);
    const float actual_e = current_position.e;
    current_position.e += length / planner.e_factor[active_extruder];
    Notifier_POS_E N(ClientFSM::Load_unload, GetPhaseIndex(phase), actual_e, current_position.e, progress_min, progress_max);
    line_to_current_position(fr_mm_s);
    planner.synchronize();
}

void Pause::plan_e_move(const float &length, const feedRate_t &fr_mm_s) {
    current_position.e += length / planner.e_factor[active_extruder];
    while (!planner.buffer_line(current_position, fr_mm_s, active_extruder)) {
        delay(50);
    }
}

void Pause::plan_e_move_notify_progress(const float &length, const feedRate_t &fr_mm_s, PhasesLoadUnload phase, uint8_t progress_min, uint8_t progress_max) {
    const float actual_e = current_position.e;
    current_position.e += length / planner.e_factor[active_extruder];
    Notifier_POS_E N(ClientFSM::Load_unload, GetPhaseIndex(phase), actual_e, current_position.e, progress_min, progress_max);
    while (!planner.buffer_line(current_position, fr_mm_s, active_extruder)) {
        delay(50);
    }
}

// Start the heater idle timers
void Pause::hotend_idle_start(uint32_t time) {
    HOTEND_LOOP()
    thermalManager.hotend_idle[e].start((millis_t)(time)*1000UL);
}

bool Pause::CanSafetyTimerExpire() const {
    return canSafetyTimerExpire(phase);
}
bool Pause::canSafetyTimerExpire(PhasesLoadUnload phase) {
    switch (phase) {
    case PhasesLoadUnload::UserPush: //add more
        return false;
    default:
        return true;
    }
}

void Pause::setPhase(PhasesLoadUnload ph, uint8_t progress_tot, uint8_t progress) {
    phase = ph;
    fsm_change(ClientFSM::Load_unload, phase, progress_tot, progress);
}

/**
 * Load filament into the hotend
 *
 * - Fail if the a safe temperature was not reached
 * - If pausing for confirmation, wait for a click or M108
 * - Show "wait for load" placard
 * - Load and purge filament
 * - Show "Purge more" / "Continue" menu
 * - Return when "Continue" is selected
 *
 * Returns 'true' if load was completed, 'false' for abort
 */
bool Pause::FilamentLoad() {

    // actual temperature does not matter, only target
    if (!is_target_temperature_safe())
        return false;

#if ENABLED(PID_EXTRUSION_SCALING)
    bool extrusionScalingEnabled = thermalManager.getExtrusionScalingEnabled();
    thermalManager.setExtrusionScalingEnabled(false);
#endif //ENABLED(PID_EXTRUSION_SCALING)

    AutoRestore<float> AR(planner.settings.retract_acceleration);
    LoadPhases_t load_ph = LoadPhases_t::_init;

    bool ret = true;
    const float purge_ln = std::max(purge_length, minimal_purge);

    do {

        //transitions
        switch (load_ph) {
        case LoadPhases_t::_init:
        case LoadPhases_t::has_slow_load:
            if (slow_load_length > 0) {
                load_ph = LoadPhases_t::check_filament_sensor;
            } else {
                load_ph = LoadPhases_t::wait_temp;
            }
            break;
        case LoadPhases_t::check_filament_sensor:
            if (fs_get_state() == fsensor_t::NoFilament) {
                fsm_change(ClientFSM::Load_unload, PhasesLoadUnload::MakeSureInserted, 0, 0);
            } else {
                fsm_change(ClientFSM::Load_unload, PhasesLoadUnload::UserPush, 0, 0);
                load_ph = LoadPhases_t::user_push__ask;
            }
            break;
        case LoadPhases_t::user_push__ask:
            if (ClientResponseHandler::GetResponseFromPhase(PhasesLoadUnload::UserPush) == Response::Continue) { // TODO method without param using actual phase
                load_ph = LoadPhases_t::load_in_gear;
            } else {
                load_ph = LoadPhases_t::check_filament_sensor;
            }
            break;
        case LoadPhases_t::load_in_gear: //slow load
        {
            AutoRestore<bool> CE(thermalManager.allow_cold_extrude);
            thermalManager.allow_cold_extrude = true;
            do_e_move_notify_progress(slow_load_length, FILAMENT_CHANGE_SLOW_LOAD_FEEDRATE, PhasesLoadUnload::Inserting, 10, 30); // TODO method without param using actual phase
            set_filament(filament_to_load);
        }
            load_ph = LoadPhases_t::wait_temp;
            break;
        case LoadPhases_t::wait_temp:
            if (ensure_safe_temperature_notify_progress(PhasesLoadUnload::WaitingTemp, 30, 50)) {
                load_ph = LoadPhases_t::long_load;
            } else {
                load_ph = LoadPhases_t::error_temp;
            }
            break;
        case LoadPhases_t::error_temp:
            ret = false;
            load_ph = LoadPhases_t::_finish;
            break;
        case LoadPhases_t::has_long_load:
            if (fast_load_length) {
                load_ph = LoadPhases_t::long_load;
            } else {
                load_ph = LoadPhases_t::stand_alone_purge;
            }
            break;
        case LoadPhases_t::long_load:
            planner.settings.retract_acceleration = FILAMENT_CHANGE_FAST_LOAD_ACCEL;
            do_e_move_notify_progress(fast_load_length, FILAMENT_CHANGE_FAST_LOAD_FEEDRATE, PhasesLoadUnload::Loading, 50, 70);
            load_ph = LoadPhases_t::purge;
            break;
        case LoadPhases_t::purge:
            // Extrude filament to get into hotend
            do_e_move_notify_progress(purge_ln, ADVANCED_PAUSE_PURGE_FEEDRATE, PhasesLoadUnload::Purging, 70, 99);
            setPhase(PhasesLoadUnload::IsColor, 99);
            load_ph = LoadPhases_t::ask_is_color_correct;
            break;
        case LoadPhases_t::stand_alone_purge:
            // Extrude filament to get into hotend
            do_e_move_notify_progress(purge_ln, ADVANCED_PAUSE_PURGE_FEEDRATE, PhasesLoadUnload::Purging, 70, 99);
            setPhase(PhasesLoadUnload::IsColorPurge, 99);
            load_ph = LoadPhases_t::ask_is_color_correct__stand_alone_purge;
            break;
        case LoadPhases_t::ask_is_color_correct: {
            const Response response = ClientResponseHandler::GetResponseFromPhase(PhasesLoadUnload::IsColor);
            if (response == Response::Purge_more) {
                load_ph = LoadPhases_t::purge;
            }
            if (response == Response::Retry) {
                load_ph = LoadPhases_t::eject;
            }
            if (response == Response::Continue) {
                load_ph = LoadPhases_t::_finish;
            }
        } break;
        case LoadPhases_t::ask_is_color_correct__stand_alone_purge: {
            const Response response = ClientResponseHandler::GetResponseFromPhase(PhasesLoadUnload::IsColorPurge);
            if (response == Response::Purge_more) {
                load_ph = LoadPhases_t::stand_alone_purge;
            }
            if (response == Response::Continue) {
                load_ph = LoadPhases_t::_finish;
            }
        } break;
        case LoadPhases_t::eject:
            do_e_move_notify_progress(-slow_load_length - fast_load_length - purge_ln, FILAMENT_CHANGE_FAST_LOAD_FEEDRATE, PhasesLoadUnload::Ejecting, 10, 99);
            load_ph = LoadPhases_t::has_slow_load;
            break;
        default:
            load_ph = LoadPhases_t::_finish;
        }

        idle(true); // idle loop to prevet wdt and manage heaters etc, true == do not shutdown steppers

    } while (load_ph != LoadPhases_t::_finish);

#if ENABLED(PID_EXTRUSION_SCALING)
    thermalManager.setExtrusionScalingEnabled(extrusionScalingEnabled);
#endif //ENABLED(PID_EXTRUSION_SCALING)

    return ret;
}

/**
 * Unload filament from the hotend
 *
 * - Fail if the a safe temperature was not reached
 * - Show "wait for unload" placard
 * - Retract, pause, then unload filament
 * - Disable E stepper (on most machines)
 *
 * Returns 'true' if unload was completed, 'false' for abort
 */
bool Pause::FilamentUnload() {

    if (!ensure_safe_temperature_notify_progress(PhasesLoadUnload::WaitingTemp, 0, 50)) {
        return false;
    }

#if ENABLED(PID_EXTRUSION_SCALING)
    bool extrusionScalingEnabled = thermalManager.getExtrusionScalingEnabled();
    thermalManager.setExtrusionScalingEnabled(false);
#endif //ENABLED(PID_EXTRUSION_SCALING)

    static const RamUnloadSeqItem ramUnloadSeq[] = FILAMENT_UNLOAD_RAMMING_SEQUENCE;
    decltype(RamUnloadSeqItem::e) ramUnloadLength = 0; //Sum of ramming distances starting from first retraction

    constexpr float mm_per_minute = 1 / 60.f;
    constexpr size_t ramUnloadSeqSize = sizeof(ramUnloadSeq) / sizeof(RamUnloadSeqItem);

    //cannot draw progress in plan_e_move, so just change phase to ramming
    setPhase(PhasesLoadUnload::Ramming, 50, 0);
    {
        bool counting = false;
        for (size_t i = 0; i < ramUnloadSeqSize; ++i) {
            plan_e_move(ramUnloadSeq[i].e, ramUnloadSeq[i].feedrate * mm_per_minute);
            if (ramUnloadSeq[i].e < 0)
                counting = true;
            if (counting)
                ramUnloadLength += ramUnloadSeq[i].e;
        }
    }

    // Unload filament
    const float saved_acceleration = planner.settings.retract_acceleration;
    planner.settings.retract_acceleration = FILAMENT_CHANGE_UNLOAD_ACCEL;

    planner.synchronize(); //do_pause_e_move(0, (FILAMENT_CHANGE_UNLOAD_FEEDRATE));//do previous moves, so Ramming text is visible

    // subtract the already performed extruder movement from the total unload length
    do_e_move_notify_progress((unload_length - ramUnloadLength), (FILAMENT_CHANGE_UNLOAD_FEEDRATE), PhasesLoadUnload::Unloading, 51, 99);

    planner.settings.retract_acceleration = saved_acceleration;

    set_filament(FILAMENT_NONE);

// Disable E steppers for manual change
#if HAS_E_STEPPER_ENABLE
    disable_e_stepper(active_extruder);
    safe_delay(100);
#endif

#if ENABLED(PID_EXTRUSION_SCALING)
    thermalManager.setExtrusionScalingEnabled(extrusionScalingEnabled);
#endif //ENABLED(PID_EXTRUSION_SCALING)

    /// IsUnloaded confirm phase
    Response isUnloaded;
    Response manualUnload;
    do {
        setPhase(PhasesLoadUnload::IsFilamentUnloaded, 100, 100);
        do {
            idle(true);
            isUnloaded = ClientResponseHandler::GetResponseFromPhase(PhasesLoadUnload::IsFilamentUnloaded);
        } while (isUnloaded != Response::Yes && isUnloaded != Response::No);
        if (isUnloaded == Response::No) {
            setPhase(PhasesLoadUnload::ManualUnload, 100, 100);
            do {
                idle(true);
                manualUnload = ClientResponseHandler::GetResponseFromPhase(PhasesLoadUnload::ManualUnload);
            } while (manualUnload != Response::Continue);
            isUnloaded = Response::Yes;
        }
    } while (isUnloaded != Response::Yes);

    return true;
}

void Pause::park_nozzle_and_notify(const float &retract, const xyz_pos_t &park_point) {
    // Initial retract before move to filament change position
    if (retract && thermalManager.hotEnoughToExtrude(active_extruder))
        do_pause_e_move(retract, PAUSE_PARK_RETRACT_FEEDRATE);

    // Park the nozzle by moving up by z_lift and then moving to (x_pos, y_pos)
    if (!axes_need_homing()) {
        {
            Notifier_POS_Z N(ClientFSM::Load_unload, GetPhaseIndex(PhasesLoadUnload::Parking), current_position.z, park_point.z, 0, Z_MOVE_PRECENT);
            do_blocking_move_to_z(_MIN(current_position.z + park_point.z, Z_MAX_POS), NOZZLE_PARK_Z_FEEDRATE);
        }
        {
            const bool x_greater_than_y = std::abs(current_position.x - park_point.x) > std::abs(current_position.y - park_point.y);
            const float &begin_pos = x_greater_than_y ? current_position.x : current_position.y;
            const float &end_pos = x_greater_than_y ? park_point.x : park_point.y;
            if (x_greater_than_y) {
                Notifier_POS_X N(ClientFSM::Load_unload, GetPhaseIndex(PhasesLoadUnload::Parking), begin_pos, end_pos, Z_MOVE_PRECENT, 100);
                do_blocking_move_to_xy(park_point, NOZZLE_PARK_XY_FEEDRATE);
            } else {
                Notifier_POS_Y N(ClientFSM::Load_unload, GetPhaseIndex(PhasesLoadUnload::Parking), begin_pos, end_pos, Z_MOVE_PRECENT, 100);
                do_blocking_move_to_xy(park_point, NOZZLE_PARK_XY_FEEDRATE);
            }
        }
        report_current_position();
    }
}

void Pause::unpark_nozzle_and_notify() {
    // Move XY to starting position, then Z
    {
        const bool x_greater_than_y = std::abs(current_position.x - resume_position.x) > std::abs(current_position.y - resume_position.y);
        const float &begin_pos = x_greater_than_y ? current_position.x : current_position.y;
        const float &end_pos = x_greater_than_y ? resume_position.x : resume_position.y;
        if (x_greater_than_y) {
            Notifier_POS_X N(ClientFSM::Load_unload, GetPhaseIndex(PhasesLoadUnload::Unparking), begin_pos, end_pos, 0, XY_MOVE_PRECENT);
            do_blocking_move_to_xy(resume_position, NOZZLE_PARK_XY_FEEDRATE);
        } else {
            Notifier_POS_Y N(ClientFSM::Load_unload, GetPhaseIndex(PhasesLoadUnload::Unparking), begin_pos, end_pos, 0, XY_MOVE_PRECENT);
            do_blocking_move_to_xy(resume_position, NOZZLE_PARK_XY_FEEDRATE);
        }
    }
    // Move Z_AXIS to saved position
    {
        Notifier_POS_Z N(ClientFSM::Load_unload, GetPhaseIndex(PhasesLoadUnload::Unparking), current_position.z, resume_position.z, XY_MOVE_PRECENT, 100);
        do_blocking_move_to_z(resume_position.z, feedRate_t(NOZZLE_PARK_Z_FEEDRATE));
    }
}

// public:

/**
 * Pause procedure
 *
 * - Abort if already paused
 * - Send host action for pause, if configured
 * - Abort if TARGET temperature is too low
 * - Display "wait for start of filament change" (if a length was specified)
 * - Initial retract, if current temperature is hot enough
 * - Park the nozzle at the given position
 * - Call FilamentUnload (if a length was specified)
 *
 * Return 'true' if pause was completed, 'false' for abort
 */
bool Pause::PrintPause(float retract, const xyz_pos_t &park_point) {

    if (did_pause_print)
        return false; // already paused

    if (!DEBUGGING(DRYRUN) && unload_length && thermalManager.targetTooColdToExtrude(active_extruder)) {
        SERIAL_ECHO_MSG(MSG_ERR_HOTEND_TOO_COLD);

        return false; // unable to reach safe temperature
    }

    // Indicate that the printer is paused
    ++did_pause_print;

    print_job_timer.pause();

    // Save current position
    resume_position = current_position;

    // Wait for buffered blocks to complete
    planner.synchronize();

#if ENABLED(ADVANCED_PAUSE_FANS_PAUSE) && FAN_COUNT > 0
    thermalManager.set_fans_paused(true);
#endif

    park_nozzle_and_notify(retract, park_point);

    if (unload_length) // Unload the filament
        FilamentUnload();

    return true;
}

/**
 * Resume or Start print procedure
 *
 * - If not paused, do nothing and return
 * - Reset heater idle timers
 * - Load filament if specified, but only if:
 *   - a nozzle timed out, or
 *   - the nozzle is already heated.
 * - Display "wait for print to resume"
 * - Re-prime the nozzle...
 *   -  FWRETRACT: Recover/prime from the prior G10.
 * - Move the nozzle back to resume_position
 * - Sync the planner E to resume_position.e
 * - Send host action for resume, if configured
 * - Resume the current SD print job, if any
 */
void Pause::PrintResume() {

    if (!did_pause_print)
        return;

    FilamentLoad();

// Intelligent resuming
#if ENABLED(FWRETRACT)
    // If retracted before goto pause
    if (fwretract.retracted[active_extruder])
        do_pause_e_move(-fwretract.settings.retract_length, fwretract.settings.retract_feedrate_mm_s);
#endif

    unpark_nozzle_and_notify();

#if ADVANCED_PAUSE_RESUME_PRIME != 0
    do_pause_e_move(ADVANCED_PAUSE_RESUME_PRIME, feedRate_t(ADVANCED_PAUSE_PURGE_FEEDRATE));
#endif

    // Now all extrusion positions are resumed and ready to be confirmed
    // Set extruder to saved position
    planner.set_e_position_mm((destination.e = current_position.e = resume_position.e));

    --did_pause_print;

#if ENABLED(ADVANCED_PAUSE_FANS_PAUSE) && FAN_COUNT > 0
    thermalManager.set_fans_paused(false);
#endif

    // Resume the print job timer if it was running
    if (print_job_timer.isPaused())
        print_job_timer.start();

    fs_clr_sent(); //reset filament sensor M600 sent flag

#if HAS_DISPLAY
    ui.reset_status();
#endif
}
