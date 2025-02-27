# Hardware & PCB
The board is very, very compact, at just 21mm x 35mm, designed as a double-layer PCB, and integrates a smart power control system alongside motion-based UVC LED activation, and is able to last +30 days on battery.

The board features:
1- MCU: STM32L031K6T – a low-power Cortex-M0+ MCU to handle control logic efficiently.

2- Motion Sensing: LSM6DS3 – a 3D accelerometer & gyroscope determines orientation to activate the correct UVC LED.

3- UVC LEDs: 2x 254nm LEDs (23mW, 150mA) – ensure effective disinfection, switching every 60 minutes for 90 seconds based on orientation.

4- Power Management:
- L7BM06CDT (6V regulator)
- L78L33ACUTR (3.3V regulator) for the MCU
- MCP73831-2-MC (Li-ion charging IC) for battery management

5- User Interface & Control:
- Status LEDs (2x) & Power LED (1x) for feedback.
- A user button for additional interaction.
- Power Button using D Flip-Flop (74HC74) – holds power after a 5-second press, controlled via an RC delay circuit.
Instead of an always-on MCU (to save the battery), the device remains completely powered off until the user presses the power button for 5 seconds.
Uses a D flip-flop latch circuit with an RC network to ensure accidental presses don’t turn it on.
- ON/OFF Switch for direct power control when needed.
- 4-pin JST connector for ST-LINK interface
- Two 2-pin JST connector for the batteries and their charger

# Software
# **STM32 UVC LED Control with Low Power Mode**

## **Overview**

This project controls **UVC LEDs** based on the device's **orientation**, using an **LSM6DS33 accelerometer**. It follows a power-efficient cycle:

1. **Waits 30 seconds** before activating.
2. **Blinks an indicator LED for 5 seconds**.
3. **Determines the orientation** of the device.
4. **Activates the appropriate UVC LED** for **90 seconds** while the indicator LED blinks.
5. **Turns OFF all LEDs** after operation.
6. **Enters deep sleep mode (STOP mode) for 1 hour** to conserve power.

The system **wakes up automatically after 1 hour** and repeats the cycle.

---

## **Hardware Requirements**

To test this code, you need:

- **STM32 (e.g., STM32F103RB Nucleo or similar)**
- **LSM6DS33 Accelerometer (I2C)**
- **UVC LEDs** (connected to GPIOA)
- **Indicator LEDs**
- **Power supply** (battery or USB)

---

## **Pin Configuration**

| **Component**            | **GPIO Pin** | **Port** |
| ------------------------ | ------------ | -------- |
| **Top UVC LED**          | `GPIO_PIN_0` | `GPIOA`  |
| **Bottom UVC LED**       | `GPIO_PIN_1` | `GPIOA`  |
| **Top Indicator LED**    | `GPIO_PIN_3` | `GPIOA`  |
| **Bottom Indicator LED** | `GPIO_PIN_7` | `GPIOA`  |
| **I2C SDA**              | `GPIO_PIN_9` | `GPIOB`  |
| **I2C SCL**              | `GPIO_PIN_8` | `GPIOB`  |

---

## **Project Structure**

```
/STM32_UVC_LowPower/
│── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── LSM6DS33.h
│   ├── Src/
│   │   ├── main.c
│   │   ├── LSM6DS33.c
│── README.md
```

---

## **How It Works**

### **1- Initialization**

- **STM32 peripherals** (GPIO, I2C, System Clock) are initialized.
- **LSM6DS33 accelerometer** is configured.

### **2- LED Activation Sequence**

- **Waits 30 seconds** after boot.
- **Blinks indicator LED** for **5 seconds**.
- **Reads orientation** from **LSM6DS33**.
- **Turns ON the correct UVC LED** (based on orientation).
- **Blinks indicator LED for 90 seconds** while UVC LED is ON.
- **Turns OFF all LEDs** after the cycle.

### **3- Power Saving Mode**

- **Enters STOP mode (Deep Sleep) for 1 hour.**
- **Disables unused peripherals (GPIO, I2C) before sleep.**
- **Resumes peripherals after waking up.**
- **Restores system clock to normal operation.**
- **Repeats the process indefinitely.**

---

## **How to Test**

### **🔹 Option 1: Flash and Run**

1. **Flash the firmware** using STM32CubeProgrammer or an ST-Link.
2. **Connect serial monitor** to see debug messages (optional).
3. **Observe LED behavior**:
   - Indicator LED blinks for 5 seconds.
   - UVC LED turns ON based on orientation.
   - Indicator LED blinks for 90 seconds while UVC is ON.
   - After LEDs turn OFF, the system **enters low-power mode for 1 hour**.
   - The process repeats.

### **🔹 Option 2: Modify Sleep Duration for Testing**

Since **1-hour sleep is long**, modify `enter_low_power_mode()` for quicker tests:

```c
while ((HAL_GetTick() - tickstart) < 30000)  // 30 seconds instead of 1 hour
```

This allows testing the **wake-up cycle faster**.

---

## **Power Consumption & Optimization**

| **Mode**                                     | **Power Consumption** |
| -------------------------------------------- | --------------------- |
| **Active (LEDs ON, CPU running)**            | \~20mA                |
| **Deep Sleep (STOP Mode, LEDs OFF)**         | **\~200-500µA**       |
| **Estimated Battery Life (1000mAh battery)** | \~66 Days             |

🔹 **Further Power Optimizations:**

- Use **Standby Mode (\~1µA) instead of STOP mode**.
- **Disable more peripherals** (e.g., ADC, UART, timers).
- Use an **RTC wake-up instead of HAL\_GetTick().**

---

## **Troubleshooting**

### **1- STM32 Doesn’t Wake Up**

**Solution: Ensure SysTick timer is active**

- **HAL\_SuspendTick()** prevents unwanted wake-ups.
- **`SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;`** is required for STOP mode.

### **2- I2C Stops Working After Wake-up**

**Solution: Reset I2C after STOP mode**

```c
__HAL_RCC_I2C1_FORCE_RESET();
HAL_Delay(10);
__HAL_RCC_I2C1_RELEASE_RESET();
MX_I2C1_Init();
LSM6DS33_init();
```

### **3- UVC LED Doesn't Turn ON**

**Solution: Check orientation detection**

- Add debug prints:

```c
printf("Orientation: %d
", orientation);
```

---

## **Additional Resources**

- [STM32 HAL Documentation](https://www.st.com/en/embedded-software/stm32cube-mcu-packages.html)
- [LSM6DS33 Datasheet](https://www.st.com/resource/en/datasheet/lsm6ds33.pdf)
- [Low Power Modes on STM32](https://www.st.com/resource/en/application_note/an4830-lowpower-modes-for-stm32-mcus.pdf)


