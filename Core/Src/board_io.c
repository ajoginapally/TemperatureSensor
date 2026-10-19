#include "board_io.h"
#include "app_config.h"

I2C_HandleTypeDef app_i2c;
UART_HandleTypeDef app_uart;

void Board_Init(void)
{
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CONFIG(RCC_I2C1CLKSOURCE_PCLK1);
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Alternate = GPIO_AF6_I2C1;
    HAL_GPIO_Init(GPIOA, &gpio);
    app_i2c.Instance = I2C1;

    app_i2c.Init.Timing = 0x00C23B4AU;
    app_i2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    app_i2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    app_i2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    app_i2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    app_i2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&app_i2c) != HAL_OK ||
        HAL_I2CEx_ConfigAnalogFilter(&app_i2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK ||
        HAL_I2CEx_ConfigDigitalFilter(&app_i2c, 0) != HAL_OK)
    {
        Error_Handler();
    }

    gpio.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Alternate = GPIO_AF1_USART2;
    HAL_GPIO_Init(GPIOA, &gpio);
    app_uart.Instance = USART2;
    app_uart.Init.BaudRate = 115200;
    app_uart.Init.WordLength = UART_WORDLENGTH_8B;
    app_uart.Init.StopBits = UART_STOPBITS_1;
    app_uart.Init.Parity = UART_PARITY_NONE;
    app_uart.Init.Mode = UART_MODE_TX_RX;
    app_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    app_uart.Init.OverSampling = UART_OVERSAMPLING_16;
    app_uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    app_uart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    if (HAL_UART_Init(&app_uart) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_NVIC_SetPriority(USART2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void Board_SetLeds(uint8_t mask)
{
#if !APP_LED_OUTPUTS_ENABLED
    mask = 0;
#endif

    GPIOB->BSRR = ((uint32_t)(~mask & 7U) << 16) | (mask & 7U);
}
