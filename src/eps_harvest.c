#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include <stdlib.h>
#include "eps_telemetry.h"


void vSolarHarvest_Task(void *pvParameters){
    uint32_t cycle_counter = 0;
    bool is_in_eclipse = false;
    float internal_temp = 25.0f; // Start at room temp

    while(1){
        cycle_counter++;

        // 1. ENVIRONMENT: Toggle Eclipse every 60 seconds
        if(cycle_counter % 60 == 0) { 
            is_in_eclipse = !is_in_eclipse;
            printf("\n--- ENV: %s ---\n", is_in_eclipse ? "ENTERING SHADOW" : "ENTERING SUNLIGHT");
        }

        if(xSemaphoreTake(xEPSDataMutex, portMAX_DELAY) == pdTRUE){
            
            // 2. SOLAR INPUT: No more rand(). 5.5W in sun, 0W in shadow.
            g_eps_telemetry.f_SolarInputPower = is_in_eclipse ? 0.0f : 5.5f;

            // 3. LOAD CALCULATION (Your original logic)
            float f_TotalLoad = 1.0f; // Essential Comms
            
            // If Payload bit is SET, add its load
            if (g_eps_telemetry.ul_PDU_Register & PDU_LOAD_PAYLOAD_BIT) {
                f_TotalLoad += 5.0f; 
            }

            // 4. THERMAL LOGIC (The new part)
            if (is_in_eclipse) {
                internal_temp -= 0.05f; // Cooling down in shadow
            } else {
                internal_temp += 0.02f; // Warming up in sun
            }

            // If HEATER bit is SET, add load AND increase temp
            if (g_eps_telemetry.ul_PDU_Register & PDU_LOAD_HEATER_BIT) {
                f_TotalLoad += 3.0f; 
                internal_temp += 0.12f; // Heater fights the cold
            }
            
            // 5. VOLTAGE MATH (Your original logic)
            float f_NetPower = g_eps_telemetry.f_SolarInputPower - f_TotalLoad;
            g_eps_telemetry.f_BusVoltage += f_NetPower * 0.01f;
            
            xSemaphoreGive(xEPSDataMutex);

            // 6. VERIFICATION (Updated to show Temp)
            printf("HARVEST: Solar: %.1fW | Load: %.1fW | Temp: %.2fC | V: %.2fV\n", 
                    g_eps_telemetry.f_SolarInputPower, f_TotalLoad, internal_temp, g_eps_telemetry.f_BusVoltage);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
