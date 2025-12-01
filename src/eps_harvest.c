#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "eps_telemetry.h"


void vSolarHarvest_Task(void *pvParameters){
    srand((unsigned int)time(NULL));

    while(1){
        // 1. Simulate Solar Power Input
        // Generate a random power input between 0.0W and 10.0W
        float simulated_power = (float)(rand() % 1000) / 100.0f;

        // 2. Safely Update the Shared Telemetry Data
        // The Mutex must be acquired BEFORE accessing or modifying the shared data.
        if(xSemaphoreTake(xEPSDataMutex, portMAX_DELAY) == pdTRUE){
            //3. Simulate Solar Power Input (This is the random part)
            //I update this first to calculate net power
            g_eps_telemetry.f_SolarInputPower = simulated_power;

            // --- 4. Calculate Total Load (Based on PDU Register Status) ---
            // Essential Load (COMMS) is always present: 1W
            float f_TotalLoad = 1.0f; 

            // Non-Essential Load (Payload/Heater) is 3W total, but only active if the bits are SET.
            // Check if the PAYLOAD bit or the HEATER bit are currently ON in the PDU Register:
            if ((g_eps_telemetry.ul_PDU_Register & PDU_LOAD_PAYLOAD_BIT) || 
                (g_eps_telemetry.ul_PDU_Register & PDU_LOAD_HEATER_BIT)) {
                f_TotalLoad += 8.0f; 
            }
            
            // 5. Calculate Net Power and Voltage Change
            float f_NetPower = g_eps_telemetry.f_SolarInputPower - f_TotalLoad;

            // Simulate Voltage Change (change is proportional to Net Power over time)
            // 0.01f is the conversion factor representing the inverse of the battery's capacity/resistance
            g_eps_telemetry.f_BusVoltage += f_NetPower * 0.01f;
            
            // 6. Release the Mutex after the update is complete.
            xSemaphoreGive(xEPSDataMutex);

            // 7. Verification 
            printf("HARVEST: Input: %.2f W | Load: %.1f W | Net P: %.2f  W | Voltage:  %.2f V \n", g_eps_telemetry.f_SolarInputPower, f_TotalLoad, f_NetPower, g_eps_telemetry.f_BusVoltage);
        }
        else {
            printf("HARVEST: ERROR - Failed to acquire mutex!\n");
        }

        // Sleep for 1 second (this is the low-priority, slow task)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
