#ifndef BOARD_IO_H
#define BOARD_IO_H
#include "main.h"
extern I2C_HandleTypeDef app_i2c;
extern UART_HandleTypeDef app_uart;
void Board_Init(void);
void Board_SetLeds(uint8_t mask);
#endif
