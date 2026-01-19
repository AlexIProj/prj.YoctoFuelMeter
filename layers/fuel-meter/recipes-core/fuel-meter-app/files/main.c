#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "config.h"
#include "gpio_driver.h"
#include "sensors.h"
#include "serial_com.h"
#include "calcs.h" // Presupun ca ai calculele aici

// Configurări Hardware
#define BTN_OFFSET 22
#define SPD_OFFSET 17
#define FLOW_OFFSET 27

#define SERIAL_PORT "/dev/ttyACM0"
#define BAUD_RATE 115200

int main() {
    printf("--- RPi 5 Fuel Meter Starting ---\n");

    // 1. SERIAL
    int ser_fd = SerialCom_Init(SERIAL_PORT, BAUD_RATE);

    // 2. SENSORI (Viteză și Debit - Folosesc Interrupts/Events)
    // Parametrul '1' la final înseamnă că activăm detectarea hardware a fronturilor
    struct gpiod_line_request* req_speed = ConfigLineInput(GPIO_CHIP_PATH, SPD_OFFSET, "speed_sensor", 1);
    struct gpiod_line_request* req_flow  = ConfigLineInput(GPIO_CHIP_PATH, FLOW_OFFSET, "flow_sensor", 1);

    pthread_t t_speed, t_flow;
    pthread_create(&t_speed, NULL, Thread_CountSpeed, (void*)req_speed);
    pthread_create(&t_flow,  NULL, Thread_CountFlow,  (void*)req_flow);

    // 3. BUTON (Simplu Input - Fără Events Hardware)
    // Parametrul '0' la final înseamnă input simplu (polling), nu interrupts.
    // Este mai ușor de gestionat toggle-ul manual în main loop.
    struct gpiod_line_request* req_btn = ConfigLineInput(GPIO_CHIP_PATH, BTN_OFFSET, "btn_toggle", 0);

    // Variabile Stare
    int metric_mode = 1;      // 1 = L/100km, 0 = MPG
    int last_btn_state = 0;   // Memorează starea anterioară a butonului
    int display_timer = 0;    // Divizor de timp pentru afișare
    char buffer[64];

    // MAIN LOOP - Rulează la fiecare 100ms (0.1 secunde)
    while(1) {
        // --- A. LOGICA DE BUTON (TOGGLE) ---
        int current_btn_state = GPIOD_GetLineValue(req_btn, BTN_OFFSET);

        // DETECTARE FRONT CRESCĂTOR (Rising Edge)
        // Dacă acum e 1, dar tura trecută era 0 -> S-a apăsat acum!
        if (current_btn_state == 1 && last_btn_state == 0) {
            metric_mode = !metric_mode; // Schimbă 1->0 sau 0->1
            printf(">> BUTTON PRESSED! Changing mode to: %s\n", metric_mode ? "Metric" : "Imperial");
            
            // Forțăm actualizarea afișajului imediat la apăsare
            display_timer = 10; 
        }
        
        // Salvăm starea pentru tura viitoare
        last_btn_state = current_btn_state;


        // --- B. LOGICA DE AFIȘARE (O dată pe secundă) ---
        display_timer++;
        if (display_timer >= 10) { // 10 * 100ms = 1 secundă
            display_timer = 0;

            // 1. Luăm datele
            int p_speed, p_flow;
            GetAndResetCounts(&p_speed, &p_flow);

            // 2. Calcule simple (Exemplu)
            // Aici trebuie să aplici formulele tale reale din calcs.c
            // p_speed sunt impulsuri acumulate în ultima secundă => Hz
            double speed_kmh = (double)p_speed; // * Coeficient
            double flow_lph  = (double)p_flow;  // * Coeficient

            double consumption = 0.0;
            
            if (metric_mode) {
                // Mod Metric (L/100km)
                if (speed_kmh > 0) consumption = (flow_lph / speed_kmh) * 100.0;
                snprintf(buffer, sizeof(buffer), "Consumption: %.1f L/100Km", consumption);
            } else {
                // Mod Imperial (MPG)
                double l_100km = (speed_kmh > 0) ? (flow_lph / speed_kmh) * 100.0 : 0;
                if (l_100km > 0) consumption = 235.21 / l_100km;
                else consumption = 0.0;
                snprintf(buffer, sizeof(buffer), "Consumption: %.1f mpg", consumption);
            }

            // 3. Trimitem la Arduino
            printf("[RPi] S:%d F:%d -> %s\n", p_speed, p_flow, buffer);
            SerialCom_SendString(ser_fd, buffer);
        }

        // Pauză mică (100ms) pentru a prinde butonul rapid
        usleep(100000); 
    }

    SerialCom_Close(ser_fd);
    return 0;
}