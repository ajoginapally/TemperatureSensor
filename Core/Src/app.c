#include "app.h"
#include "app_config.h"
#include "board_io.h"
#include "p3t1755.h"
#include "temperature_logic.h"

static const TemperatureThresholds thresholds = {APP_LOW_MC, APP_HIGH_MC};
static int32_t latest_mc;
static bool valid;
static bool sensor_ready;
static uint32_t next_sample;

static volatile uint8_t head, tail, rx_fault;
static uint8_t ring[128], rx_byte;
static char line[32];
static unsigned line_length;
static bool discard_line;

static unsigned TextLength(const char *text) {
  unsigned n = 0;
  while (text[n])
    ++n;
  return n;
}

static bool Equals(const char *a, const char *b) {
  while (*a && *a == *b) {
    ++a;
    ++b;
  }
  return *a == *b;
}

static void Print(const char *text) {
  (void)HAL_UART_Transmit(&app_uart, (const uint8_t *)text,
                          (uint16_t)TextLength(text), 50);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart) {
  if (uart != &app_uart)
    return;
  uint8_t next = (uint8_t)((head + 1U) & 127U);
  if (next == tail)
    rx_fault = 1;
  else {
    ring[head] = rx_byte;
    head = next;
  }
  if (HAL_UART_Receive_IT(uart, &rx_byte, 1) != HAL_OK)
    rx_fault = 1;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *uart) {
  if (uart != &app_uart)
    return;
  rx_fault = 1;

  if (uart->RxState == HAL_UART_STATE_READY)
    (void)HAL_UART_Receive_IT(uart, &rx_byte, 1);
}

static void Command(void) {
  line[line_length] = '\0';
  if (Equals(line, "help")) {
    Print("help - commands\r\ntemp - latest temperature\r\n");
  } else if (Equals(line, "temp")) {
    if (!valid)
      Print("Temperature unavailable (sensor error or warming up)\r\n");
    else {
      int32_t magnitude = latest_mc < 0 ? -latest_mc : latest_mc;

      char number[12];
      unsigned pos = 0;
      uint32_t whole = (uint32_t)magnitude / 1000U;
      do {
        number[pos++] = (char)('0' + whole % 10U);
        whole /= 10U;
      } while (whole);
      Print("Temperature: ");
      if (latest_mc < 0)
        Print("-");
      while (pos) {
        char digit[2] = {number[--pos], 0};
        Print(digit);
      }
      char fraction[8] = {'.',
                          (char)('0' + magnitude / 100 % 10),
                          (char)('0' + magnitude / 10 % 10),
                          (char)('0' + magnitude % 10),
                          ' ',
                          'C',
                          '\r',
                          '\n'};
      (void)HAL_UART_Transmit(&app_uart, (uint8_t *)fraction, sizeof fraction,
                              50);
    }
  } else if (line_length)
    Print("Unknown command. Type help.\r\n");
}

void App_Init(void) {
  Board_SetLeds(0);
  if (HAL_UART_Receive_IT(&app_uart, &rx_byte, 1) != HAL_OK)
    Error_Handler();
  Print("Temperature sensor starting. Type help or temp, then Enter.\r\n");
#if !APP_LED_OUTPUTS_ENABLED
  Print(
      "LED outputs inhibited: confirm R5-R7, then enable in app_config.h.\r\n");
#endif

  next_sample = HAL_GetTick() + 25U;
}

void App_Poll(void) {
  if (rx_fault) {

    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    tail = head;
    rx_fault = 0;
    __set_PRIMASK(primask);
    discard_line = true;
    line_length = 0;
    Print("UART input lost. Press Enter, then retry.\r\n");
  }

  for (unsigned n = 0; n < 128 && tail != head; ++n) {
    char c = (char)ring[tail];
    tail = (uint8_t)((tail + 1U) & 127U);
    if (c == '\r' || c == '\n') {
      if (!discard_line)
        Command();
      line_length = 0;
      discard_line = false;
    } else if (!discard_line) {
      if (c == '\b' || c == 127) {
        if (line_length)
          --line_length;
      } else if (c >= 32 && c <= 126) {
        if (line_length < sizeof line - 1)
          line[line_length++] = c;
        else {
          discard_line = true;
          Print("Command too long.\r\n");
        }
      }
    }
  }
  uint32_t now = HAL_GetTick();
  if ((int32_t)(now - next_sample) < 0)
    return;
  next_sample = now + APP_SAMPLE_MS;
  if (!sensor_ready) {

    uint8_t config = 0x28;
    sensor_ready =
        HAL_I2C_Mem_Write(&app_i2c, APP_SENSOR_ADDRESS << 1, 1,
                          I2C_MEMADD_SIZE_8BIT, &config, 1, 25) == HAL_OK;
    valid = false;
    Board_SetLeds(0);
    if (!sensor_ready)
      Print("Sensor configuration failed; retrying.\r\n");

    return;
  }
  uint8_t raw[2];
  valid = HAL_I2C_Mem_Read(&app_i2c, APP_SENSOR_ADDRESS << 1, 0,
                           I2C_MEMADD_SIZE_8BIT, raw, 2, 25) == HAL_OK;
  if (valid)
    latest_mc = P3T1755_Decode(raw[0], raw[1]);
  else {
    sensor_ready = false;
    Print("Sensor read failed; retrying.\r\n");
  }
  Board_SetLeds(
      Temperature_LedMask(Temperature_Classify(latest_mc, valid, &thresholds)));
}
