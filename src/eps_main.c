/*
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "eps_telemetry.h"
#include "esp_log.h"

EPS_Telemetry_t g_eps_telemetry = {
    .f_BusVoltage = 15.0f, // Start at a high voltage
    .f_StateOfCharge = 50.0f,
    .f_SolarInputPower = 0.0f,
    .ul_PDU_Register = (PDU_LOAD_PAYLOAD_BIT | PDU_LOAD_HEATER_BIT | PDU_LOAD_COMMS_BIT), // All loads ON
    };

SemaphoreHandle_t xEPSDataMutex = NULL;

extern void vSolarHarvest_Task(void *pvParameters);
extern void vEPS_Manager_Task(void *pvParameters);
extern eps_mode_t current_mode;

void app_main() {
    xEPSDataMutex = xSemaphoreCreateMutex();

    if (xEPSDataMutex == NULL){
        ESP_LOGE("MAIN", "Failed to create mutex. System halt.");       
        return;
    }

    xTaskCreate(vEPS_Manager_Task, "EPS Manager", 4096, NULL, configMAX_PRIORITIES - 1, NULL);  // Highest priority
    xTaskCreate(vSolarHarvest_Task, "Solar Harvest", 2048, NULL, tskIDLE_PRIORITY + 1, NULL); // Low priority


}

*/