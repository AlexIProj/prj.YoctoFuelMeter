#include <Arduino.h>
#include "simulator.h"
#include "lcd_display.h"

FuelSimulator simulator;
DisplayLCD display;

String serial_buffer = "";
bool cmd_received = false;

// ============================================================
// SENSOR SPECIFICATIONS (from datasheet)
// ============================================================
#define SENSOR_SPEED_PULSES_PER_MILE  6000.0
#define SENSOR_FLOW_PULSES_PER_CM3    12.0

// Unit conversions
#define KM_PER_MILE     1.609344
#define CM3_PER_LITER   1000.0

// ============================================================
// SIMULATION CONFIGURATION - SET REALISTIC VALUES HERE
// ============================================================
// Vehicle speed in km/h
#define SIM_SPEED_KMH   80.0

// Fuel flow rate in L/hour
#define SIM_FLOW_LPH    20.0

// Minimum frequency for stable readings (pulses per 200ms sample)
#define MIN_PULSES_PER_SAMPLE  50

// Button toggle interval (ms) - toggles metric/imperial
#define BUTTON_TOGGLE_INTERVAL_MS 10000

// ============================================================
// AUTO-CALCULATED FREQUENCIES
// ============================================================
// Convert km/h to pulses/second:
// speed_hz = (km/h) / (km/mile) * (pulses/mile) / 3600
// speed_hz = (km/h) * (pulses/mile) / (km/mile) / 3600
#define CALC_SPEED_HZ  ((SIM_SPEED_KMH / KM_PER_MILE) * SENSOR_SPEED_PULSES_PER_MILE / 3600.0)

// Convert L/hour to pulses/second:
// flow_hz = (L/h) * (cm3/L) * (pulses/cm3) / 3600
#define CALC_FLOW_HZ   ((SIM_FLOW_LPH * CM3_PER_LITER * SENSOR_FLOW_PULSES_PER_CM3) / 3600.0)

// Calculate scale factor to ensure minimum pulses per sample
// Sample period is 200ms = 0.2s, so min_hz = MIN_PULSES_PER_SAMPLE / 0.2
#define MIN_HZ_FOR_STABILITY  (MIN_PULSES_PER_SAMPLE / 0.2)

// Scale factor: ensure the lower frequency meets minimum
#define SCALE_FACTOR_SPEED  (CALC_SPEED_HZ < MIN_HZ_FOR_STABILITY ? (MIN_HZ_FOR_STABILITY / CALC_SPEED_HZ) : 1.0)
#define SCALE_FACTOR_FLOW   (CALC_FLOW_HZ < MIN_HZ_FOR_STABILITY ? (MIN_HZ_FOR_STABILITY / CALC_FLOW_HZ) : 1.0)
#define SCALE_FACTOR        (SCALE_FACTOR_SPEED > SCALE_FACTOR_FLOW ? SCALE_FACTOR_SPEED : SCALE_FACTOR_FLOW)

// Final frequencies (scaled for stability, ratio preserved)
// Use rounding instead of truncation to preserve ratio accuracy
#define SIM_SPEED_HZ   ((long)(CALC_SPEED_HZ * SCALE_FACTOR + 0.5))
#define SIM_FLOW_HZ    ((long)(CALC_FLOW_HZ * SCALE_FACTOR + 0.5))

// Expected consumption calculation for verification
// Consumption (L/100km) = (L/h) / (km/h) * 100
#define EXPECTED_CONSUMPTION  ((SIM_FLOW_LPH / SIM_SPEED_KMH) * 100.0)

void setup() {
    Serial.begin(115200);
    serial_buffer.reserve(50);

    simulator.Setup();
    #ifdef LCD_ENABLED
    Serial.println("[LCD] Initializing...");
    display.Setup();
    Serial.println("[LCD] Setup complete");
    display.ShowChar(0, 'F', false);
    display.ShowChar(1, 'U', false);
    display.ShowChar(2, 'E', false);
    display.ShowChar(3, 'L', false);
    Serial.println("[LCD] Showing FUEL...");
    display.Update();
    Serial.println("[LCD] Update done, waiting 2s...");
    delay(2000);
    display.clear();
    Serial.println("[LCD] Cleared");
    #else
    Serial.println("[LCD] DISABLED - LCD_ENABLED not defined");
    #endif

    // Start with configured values
    String initCmd = "S" + String(SIM_SPEED_HZ) + "F" + String(SIM_FLOW_HZ);
    simulator.ParseCommand(initCmd);

    Serial.println("=================================");
    Serial.println("FUEL METER SIMULATOR STARTED");
    Serial.println("=================================");
    Serial.println("Configuration:");
    Serial.print("  Speed: "); Serial.print(SIM_SPEED_KMH); Serial.println(" km/h");
    Serial.print("  Flow:  "); Serial.print(SIM_FLOW_LPH); Serial.println(" L/h");
    Serial.print("  Expected consumption: "); Serial.print(EXPECTED_CONSUMPTION, 1); Serial.println(" L/100km");
    Serial.println("Frequencies (auto-scaled for stability):");
    Serial.print("  Speed: "); Serial.print(SIM_SPEED_HZ); Serial.println(" Hz");
    Serial.print("  Flow:  "); Serial.print(SIM_FLOW_HZ); Serial.println(" Hz");
    Serial.print("  Scale factor: "); Serial.println(SCALE_FACTOR, 1);
    Serial.print("Button toggle every: "); Serial.print(BUTTON_TOGGLE_INTERVAL_MS/1000); Serial.println(" seconds");
    Serial.println("=================================");
}

void updateDisplay(String input) {
    input.trim();
    if (input.length() < 2) return;

    char unit = input.charAt(0);

    // Validate unit character - must be 'L' or 'G'
    if (unit != 'L' && unit != 'G') {
        Serial.print("[RX] Invalid unit char: ");
        Serial.println((int)unit);
        return;
    }

    String valStr = input.substring(1);
    float val = valStr.toFloat();

    // Validate value range (0-99.9)
    if (val < 0 || val > 99.9) {
        Serial.print("[RX] Invalid value: ");
        Serial.println(val);
        return;
    }

    int tens = ((int)val / 10) % 10;
    int ones = ((int)val) % 10;
    int decimal = (int)(val * 10) % 10;

    #ifdef LCD_ENABLED
    display.ShowChar(0, unit, false);
    display.ShowChar(1, (char)('0' + tens), false);
    display.ShowChar(2, (char)('0' + ones), true);
    display.ShowChar(3, (char)('0' + decimal), false);
    display.Update();
    #endif
}

void loop() {
    simulator.Loop();

    // Auto button toggle every 10 seconds
    simulator.UpdateAutoToggle(BUTTON_TOGGLE_INTERVAL_MS);

    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n') {
            cmd_received = true;
        } else if (c == '\r') {
            // Ignore carriage return
        } else if (c >= ' ' && c <= '~') {
            // Only accept printable ASCII characters
            serial_buffer += c;
        }
    }

    if (cmd_received) {
        if (serial_buffer.length() > 0) {
            Serial.print("[RX] Display cmd: ");
            Serial.println(serial_buffer);
            updateDisplay(serial_buffer);
        }
        serial_buffer = "";
        cmd_received = false;
    }
}
