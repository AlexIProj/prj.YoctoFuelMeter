#include "simulator.h"

// Definim pinii hardcodat pentru siguranță (conform config.h-ul tau: PB0=8, PB1=9)
#define PIN_SPEED_ARDUINO 9
#define PIN_FLOW_ARDUINO  8
#define PIN_BTN_ARDUINO   7

FuelSimulator::FuelSimulator() {
    _target_speed_hz = 0;
    _target_flow_hz  = 0;

    _prev_micros_speed = 0;
    _prev_micros_flow  = 0;
    
    // Stări locale pentru pini (ca să știm dacă sunt HIGH sau LOW)
    _state_speed = LOW;
    _state_flow = LOW;

    _btn_start_time = 0;
    _btn_active = false;

    _last_toggle_time = 0;
}

void FuelSimulator::Setup(void) {
    // Folosim funcțiile standard Arduino - sunt mult mai sigure
    pinMode(PIN_SPEED_ARDUINO, OUTPUT);
    pinMode(PIN_FLOW_ARDUINO, OUTPUT);
    pinMode(PIN_BTN_ARDUINO, OUTPUT);

    digitalWrite(PIN_SPEED_ARDUINO, LOW);
    digitalWrite(PIN_FLOW_ARDUINO, LOW);
    // Butonul e activ pe HIGH in simularea ta, deci pornim pe LOW
    digitalWrite(PIN_BTN_ARDUINO, LOW); 
}

void FuelSimulator::Loop(void) {
    unsigned long currentMicros = micros();

    // --- LOGICA PENTRU FLUX (PIN 8) ---
    if (_target_flow_hz > 0) {
        // Perioada completă = 1000000 / Hz. Toggle trebuie făcut la jumătate.
        unsigned long interval_flow = 1000000UL / (_target_flow_hz * 2);

        if (currentMicros - _prev_micros_flow >= interval_flow) {
            _prev_micros_flow = currentMicros;
            
            // Inversăm starea (Toggle)
            _state_flow = !_state_flow;
            digitalWrite(PIN_FLOW_ARDUINO, _state_flow);
        }
    }
    else {
        _state_flow = LOW;
        digitalWrite(PIN_FLOW_ARDUINO, LOW);
    }

    // --- LOGICA PENTRU VITEZĂ (PIN 9) ---
    if (_target_speed_hz > 0) {
        unsigned long interval_speed = 1000000UL / (_target_speed_hz * 2);

        if (currentMicros - _prev_micros_speed >= interval_speed) {
            _prev_micros_speed = currentMicros;
            
            // Inversăm starea (Toggle)
            _state_speed = !_state_speed;
            digitalWrite(PIN_SPEED_ARDUINO, _state_speed);
        }
    }
    else {
        _state_speed = LOW;
        digitalWrite(PIN_SPEED_ARDUINO, LOW);
    }

    // --- LOGICA PENTRU BUTON (PIN 7) ---
    if (_btn_active && (millis() - _btn_start_time > 200)){
        digitalWrite(PIN_BTN_ARDUINO, LOW);
        _btn_active = false;
    }
}

void FuelSimulator::TriggerButton(void) {
    digitalWrite(PIN_BTN_ARDUINO, HIGH);
    _btn_start_time = millis();
    _btn_active = true;
}

void FuelSimulator::ParseCommand(String cmd) {
    // Adaugă un debug pe Serial să fim siguri că primește comanda
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