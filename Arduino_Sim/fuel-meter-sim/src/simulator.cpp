#include "simulator.h"

FuelSimulator::FuelSimulator() {
    _target_speed_hz = 0;
    _target_flow_hz  = 0;

    _prev_micros_speed = 0;
    _prev_micros_flow  = 0;

    _btn_start_time = 0;
    _btn_active = false;

    _last_toggle_time = 0;
}

void FuelSimulator::Setup(void) {
    SIM_PORT_DDR |= (1 << PIN_SPEED_BIT) | (1 << PIN_FLOW_BIT);
    BTN_PORT_DDR |= (1 << PIN_BTN_BIT);

    SIM_PORT_OUT &= ~((1 << PIN_SPEED_BIT) | (1 << PIN_FLOW_BIT));
    BTN_PORT_OUT &= ~(1 << PIN_BTN_BIT);
}

void FuelSimulator::Loop(void) {
    unsigned long currentMicros = micros();

    if (_target_flow_hz > 0) {
        unsigned long interval_flow = 1000000UL / (_target_flow_hz * 2);

        if (currentMicros - _prev_micros_flow >= interval_flow) {
            _prev_micros_flow = currentMicros;
            SIM_PORT_PIN = (1 << PIN_FLOW_BIT);
        }
    }
    else {
        SIM_PORT_OUT &= ~(1 << PIN_FLOW_BIT);
    }

    if (_target_speed_hz > 0) {
        unsigned long interval_speed = 1000000UL / (_target_speed_hz * 2);

        if (currentMicros - _prev_micros_speed >= interval_speed) {
            _prev_micros_speed = currentMicros;
            SIM_PORT_PIN = (1 << PIN_SPEED_BIT);
        }
    }
    else {
        SIM_PORT_OUT &= ~(1 << PIN_SPEED_BIT);
    }

    if (_btn_active && (millis() - _btn_start_time > 200)) {
        BTN_PORT_OUT &= ~(1 << PIN_BTN_BIT);  // Set LOW
        _btn_active = false;
    }
}

void FuelSimulator::TriggerButton(void) {
    BTN_PORT_OUT |= (1 << PIN_BTN_BIT);  // Set HIGH
    _btn_start_time = millis();
    _btn_active = true;
}

void FuelSimulator::ParseCommand(String cmd) {
    Serial.print("CMD RECEIVED: ");
    Serial.println(cmd);

    if (cmd.indexOf('T') != -1) {
        TriggerButton();
        return;
    }

    int s_index = cmd.indexOf('S');
    int f_index = cmd.indexOf('F');

    if(s_index != -1 && f_index != -1) {
        long s_val = cmd.substring(s_index + 1, f_index).toInt();
        long f_val = cmd.substring(f_index + 1).toInt();

        if (s_val >= 0) _target_speed_hz = s_val;
        if (f_val >= 0) _target_flow_hz = f_val;
        
        Serial.print("SPEED SET TO: "); Serial.println(_target_speed_hz);
        Serial.print("FLOW SET TO: "); Serial.println(_target_flow_hz);
    }
}

void FuelSimulator::UpdateAutoToggle(unsigned long interval_ms) {
    if (millis() - _last_toggle_time > interval_ms) {
        _last_toggle_time = millis();
        TriggerButton();
    }
}