# 🛰️ EPS Controller Flight Software (FSW)

This repository contains the C/FreeRTOS Flight Software (FSW) for the Electrical Power Subsystem (EPS). The firmware is built for the **ESP32** and demonstrates mission-critical power management and autonomous fault recovery for a small satellite architecture.

---

## A. System Overview

| Component | Function | Status |
| :--- | :--- | :--- |
| **System** | Electrical Power Subsystem (EPS) | Provides, manages, and distributes all power to the CubeSat. |
| **Microcontroller** | ESP32 | Target hardware for FSW execution. |
| **Primary Feature** | **Autonomous Load Shedding** | Guarantees system survival during critical undervoltage events via FDIR-lite protocols. |

### Implemented FSW Features

* **Autonomous State Machine:** Manages transitions between operational modes based on bus voltage.
* **Low-Level Control:** Uses **Bit Masking** to control the simulated Power Distribution Unit (PDU) register.
* **Concurrent Execution:** Utilizes FreeRTOS Tasks and Mutexes for safe, deterministic processing.

---

## B. Technical Details

### Architecture Summary

The system is built on an event-driven architecture using FreeRTOS, centered around the safety of shared data.

#### **High-Level Data Flow (Tasks & Concurrency)**

| Task | Priority | Role | FSW File |
| :--- | :--- | :--- | :--- |
| **Manager Task** | High (`configMAX_PRIORITIES - 1`) | **FDIR and State Transition.** Implements the EPS State Machine. | `src/eps_logic.c` |
| **Harvest Task** | Low (`tskIDLE_PRIORITY + 1`) | **Energy Simulation.** Calculates Net Power and updates shared telemetry. | `src/eps_harvest.c` |

* **Concurrency Safety:** All access (Read/Write) to the shared `EPS_Telemetry_t` structure is protected by a **FreeRTOS Mutex** (`xEPSDataMutex`) to prevent **race conditions**.

#### **Fault Logic (FDIR-lite)**

The system implements a critical Finite State Machine (FSM) based on the simulated Bus Voltage ($V_{Bus}$). 

* **Key Function:** `vEPS_Manager_Task()`
* **Fault Trigger:** $V_{Bus} \le \mathbf{10.0V}$ (Threshold for `LOW_VOLTAGE` mode).
* **Recovery Threshold:** $V_{Bus} \ge \mathbf{11.5V}$ (Threshold to initiate `RECOVERY` mode).

#### **Low-Level Hardware Control (Bit Masking)**

The Load Shedding command is executed by manipulating the 32-bit PDU Register (`ul_PDU_Register`).

* **Load Shedding Action:** Non-essential loads (`PAYLOAD`, `HEATER`) are **turned OFF** using **Bitwise AND (&) and NOT (~):**
    
    ```c
    PDU Register &= ~(PAYLOAD | HEATER)
    ```
    *(This clears the two bits while preserving the essential COMMS bit.)*
---

## C. Verification & Validation (V&V) Interface

The functional correctness of the FSW is proven using an external Ground Segment tool.

### V&V Tool Link
* **Ground Segment Repository:** (https://github.com/ozgunbkl/EPS_VNV_Tool)

### Telemetry Parsing
The EPS FSW outputs telemetry as formatted strings via the serial port. The V&V Tool **parses** this text stream using **Regular Expressions (Regex)** to extract numerical data and run automated checks.

| Output String Example | Data Parsed |
| :--- | :--- |
| HARVEST: Input: 4.87 W | Load: 9.0 W | ... | Voltage: 10.11 V` | `Input: 4.87`, `Load: 9.0`, `Voltage: 10.11` |

### Example FDIR Execution Log

The V&V Tool monitors the output and confirms critical PDU states.

HARVEST: Input: 1.51 W | ... | Voltage: 10.03 V [V&V ALERT] Low Voltage Detected! Voltage: 10.03V <-- V&V Tool Assertion MANAGER: PDU State (HEX) : 0x00000004 (Only COMMS should be ON) <-- FSW Bit Masking Verified


---

## D. Build & Run Instructions

1.  **Dependencies:** Ensure PlatformIO Core and the ESP-IDF framework are installed.
2.  **Build:** Navigate to the project root and run:
    ```bash
    pio run
    ```
3.  **Upload:** Connect the ESP32 and run:
    ```bash
    pio run -t upload
    ```
4.  **Monitor:** Close the PlatformIO Monitor and use the linked **V&V Tool** to monitor and validate the serial output.
