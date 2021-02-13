/**
 * @file filament_sensor.hpp
 * @author Radek Vana
 * @brief basic api of filament sensor
 * @date 2019-12-16
 */

#pragma once

#include "stdint.h"

enum class fsensor_t : uint8_t {
    NotInitialized, //enable enters this state too
    HasFilament,
    NoFilament,
    NotConnected,
    Disabled
};

//basic filament sensor api
class FSensor {
protected:
    enum class send_M600_t : uint8_t {
        on_edge = 0,
        on_level = 1,
        never = 2
    };

    struct status_t {
        uint8_t M600_sent;
        send_M600_t send_M600_on;
        uint8_t meas_cycle;
    };
    status_t status;

    bool old_fs_state;
    bool run_first;
    bool current_detect_filament_insert;

    volatile fsensor_t state;
    volatile fsensor_t last_state;

    void init();
    void set_state(fsensor_t st);

    void restore_send_M600_on(FSensor::send_M600_t send_M600_on);
    send_M600_t getM600_send_on_and_disable();

    void injectM600();
    void evaluate_M600_conditions();
    void autoload_loop();

    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual void cycle() = 0;

public:
    void Cycle();
    //thread safe functions
    fsensor_t Get();
    bool DidRunOut(); //for arduino / marlin

    //switch behavior when M600 should be send
    void M600_on_edge(); //default behavior
    void M600_on_level();
    void M600_never();

    //thread safe functions, but cannot be called from interrupt
    void Enable();
    void Disable();

    fsensor_t WaitInitialized();
    void ClrSent();

    //not thread safe functions
    void InitOnEdge();
    void InitOnLevel();
    void InitNever();

    //for debug
    bool WasM600_send();
    char GetM600_send_on();

    FSensor();
};

//singleton defined in childs cpp file
FSensor &FS_instance();
