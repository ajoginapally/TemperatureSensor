# Temperature sensor firmware

## Implemented

main.c calls Board_Init() after SystemClock_Config(), then App_Init(), and runs
App_Poll() continuously. Custom calls and the UART IRQ handler live in USER CODE
blocks. Existing system clock configuration stays at 12 MHz.

- board_io.c configures GPIO, I2C1 and USART2 using ST HAL.
- app.c samples the P3T1755 every 500 ms and handles help/temp UART commands.
- p3t1755.c converts the sensor's signed 12-bit output to milli-degrees Celsius.
- temperature_logic.c classifies readings and produces LED masks.
- app_config.h contains the address, provisional thresholds and LED enable gate.

The application configures the sensor's register 0x01 to 0x28 (continuous
conversion, default 55 ms conversion time). It waits at least 25 ms before the
first bus transaction and another sample period before reading register 0x00.
Two bytes are read MSB first; the lower nibble is ignored. Negative temperatures
are sign-extended explicitly. Half-millidegree fractions are truncated toward
zero (the 0.0625 C sensor step is 62.5 milli-degrees).

A failed transaction invalidates the temperature, requests LEDs off and retries
configuration on the next period. UART never presents a failed read as fresh.
UART reception uses a 128-byte interrupt ring and a 32-byte command buffer.
Long or corrupted input is discarded through a newline. I2C calls time out in
25 ms; UART transmissions time out in 50 ms. No RTOS is required.

## Pins and settings

| Function | Pin / setting |
|---|---|
| I2C1 SCL | PA9, AF6, open drain |
| I2C1 SDA | PA10, AF6, open drain |
| I2C clock | PCLK1=12 MHz; conservative standard-mode timing (~80 kHz) |
| Pull-ups | External resistors to 3.3 V as drawn |
| LED 1 / 2 / 3 | PB0 / PB1 / PB2, active high |
| USART2 TX / RX | PA2 / PA3, AF1, provisional wiring |
| UART format | 115200 baud, 8 data bits, no parity, 1 stop bit, no flow control |
| Sensor address | 7-bit 0x49 assuming A2=0, A1=0, A0=1; shifted once for HAL |
| Cold / hot thresholds | 20 / 30 C, provisional; boundaries are normal |

Correction to earlier notes: PA8/PA9 was a screenshot misreading. The MCU pin
multiplexer supports I2C1 on PA9/PA10. Confirm actual PCB connectivity before
flashing. UART is not routed in the supplied schematic: PA2/PA3 must be accessible
and connected to a 3.3 V USB-UART adapter (TX to adapter RX, RX to adapter TX,
common ground). J-Link SWD is not itself this serial connection.

R5-R7 appear labelled 0 ohms in the schematic. GPIOs are configured, but
APP_LED_OUTPUTS_ENABLED defaults to 0 to keep outputs low. Confirm suitable
current-limiting resistors are populated, then set it to 1. The software still
computes and tests all LED patterns while outputs are inhibited.

## CubeMX ownership / regeneration

Peripheral initialization is application-owned in board_io.c, NOT generated
from the .ioc. TemperatureSensor.ioc remains the original minimal configuration;
opening it will not show these application-configured peripheral assignments.
This avoids guessing CubeMX project metadata or claiming regeneration was tested.

HAL_I2C_MODULE_ENABLED and HAL_UART_MODULE_ENABLED are in the preserved USER CODE
Header section of stm32c0xx_hal_conf.h. The I2C/UART driver and extension source
and header files were copied from your installed STM32Cube FW_C0 V1.4.1 repository,
matching the project's firmware version. Their ST license headers are retained.

Before regenerating: back up the project. Retain those four HAL .c and four .h
files and the USER CODE sections. Do not enable a second CubeMX-generated
USART2_IRQHandler alongside the application-owned one. If moving peripheral
ownership to CubeMX later, replace Board_Init with generated initialization and
adapt app_i2c/app_uart to the generated handles; do not maintain two owners.
Recalculate I2C timing when changing the clock or board rise/fall time assumptions.

## Build without hardware

CubeMX configures and generates code; STM32CubeIDE compiles and links it.
1. Open STM32CubeIDE (not CubeMX).
2. File > Import > General > Existing Projects into Workspace.
3. Select this directory and import TemperatureSensor without copying it.
4. Select the project and choose Project > Build Project (hammer icon).
5. Check the Build Console and Problems view. A successful link produces an .elf
   and a size report. The MCU has 16 KiB flash and 12 KiB RAM; check actual usage.
6. Use Build, not Run/Debug: flashing and on-target debugging require the board.

The existing .cproject builds Core and Drivers only, so host tests are excluded.
If an unoptimized Debug build exceeds flash, use size optimization (-Os) in the
project's C compiler optimization settings, then rebuild and inspect the report.
No full ARM GCC link or flash/RAM fit has been verified here: CubeIDE/ARM GCC was
not found. Application and HAL sources were compiled to Cortex-M0+ objects with
Clang; syscalls/sysmem (which require the ARM C library) were not part of that check.

Run host tests without hardware:

```sh
sh tests/run_host_tests.sh
```

These exercise sensor byte decoding, threshold boundaries, failed I2C reads,
startup/conversion delay, register/address selection, serial commands, buffer
limits, overflow recovery and LED requests using a mocked HAL. They cannot prove
pin wiring, bus timing, device addresses, display behavior or actual LED current.

## Display and remaining hardware work

UG-2864HSWEG01 identifies a 128x64 SSD1306 OLED. Display rendering is not part of
this increment. Confirm the actual four-pin breakout uses I2C, its SA0 address,
reset circuit and internal charge-pump arrangement: the PDF describes the bare
panel, including multiple possible interfaces and power configurations.

Confirm pin wiring, sensor address straps, LED resistor values and UART access.
Then test UART startup/help/temp, sensor ACK and readings, threshold patterns,
and disconnect/reconnect behavior on the real board. Display implementation and
project-lead approval remain outstanding.

Datasheet sources: local P3T1755.pdf sections 7.3.1, 7.5.1-7.5.3 and 7.5.8;
local UG-2864HSWEG01.pdf sections 1.5 and 4. CubeMX's installed STM32C031K(4-6)Tx
and GPIO-STM32C0xx pin databases supplied alternate-function assignments.
