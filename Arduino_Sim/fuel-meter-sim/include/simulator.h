#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "config.h"

class FuelSimulator {
    public:
        FuelSimulator();

        void Setup(void);
        void Loop(void);
        void ParseCommand(String command);
        void UpdateAutoToggle(unsigned long interval_ms);

    private:
        long _target_speed_hz;
        long _target_flow_hz;

        unsigned long _prev_micros_speed;
        unsigned long _prev_micros_flow;
        
        // --- ACESTEA SUNT LINIILE CARE LIPSEAU ---
        int _state_speed;
        int _state_flow;
        // -----------------------------------------

        unsigned long _btn_start_time;
        bool _btn_active;
        unsigned long _last_toggle_time;

        void TriggerButton(void);
};   

#endif