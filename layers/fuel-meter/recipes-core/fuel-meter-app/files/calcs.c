#include "calcs.h"
#include "config.h"

// ------------------ MACROS -------------------
#define MAXIMUM_FUEL_CONSUMPTION 99.9


double GetMetricConstant(void) {
    double pulses_per_km = K_SPEED_SEN / KM_PER_MILES;
    double pulses_per_liter = K_FLOW_SEN * CM3_PER_LITER;

    return (pulses_per_km / pulses_per_liter) * 100.0;
}

double GetImperialConstant(void) {
    double pulses_per_mile = K_SPEED_SEN;
    double pulses_per_gallon = K_FLOW_SEN * CM3_PER_GALLON;

    return (pulses_per_mile / pulses_per_gallon) * 100.0;
}

double CalculateFuelConsumption(int p_speed, int p_flow, int is_metric) {
    double res = 0.0, k_fuel_calc = 0.0;

    if (p_speed ==0)
        return 0.0;

    if (is_metric) {
        k_fuel_calc = GetMetricConstant();
    }
    else {
        k_fuel_calc = GetImperialConstant(); 
    }

    res = k_fuel_calc * (double)p_flow / (double)p_speed;

    return ((res <= MAXIMUM_FUEL_CONSUMPTION) ? res : MAXIMUM_FUEL_CONSUMPTION);
}