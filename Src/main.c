#include <stdint.h>

// Memory addresses for STM32F401RE registers
#define RCC_BASE        0x40023800UL
#define GPIOA_BASE      0x40020000UL
#define USART2_BASE     0x40004400UL

#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x40))

#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL      (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

#define USART2_SR       (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_DR       (*(volatile uint32_t *)(USART2_BASE + 0x04))
#define USART2_BRR      (*(volatile uint32_t *)(USART2_BASE + 0x08))
#define USART2_CR1      (*(volatile uint32_t *)(USART2_BASE + 0x0C))

void delay_ms(volatile uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 16000; i++)
    {
        __asm volatile ("nop");
    }
}

void USART2_Init(void)
{
    // Turn on GPIOA clock
    RCC_AHB1ENR |= (1U << 0);

    // Turn on USART2 clock
    RCC_APB1ENR |= (1U << 17);

    // Set PA2 to Alternate Function mode
    // PA2 is the Nucleo D1 / USART2_TX pin
    GPIOA_MODER &= ~(3U << 4);
    GPIOA_MODER |=  (2U << 4);

    // Set PA2 to AF7, which means USART2_TX
    GPIOA_AFRL &= ~(0xFU << 8);
    GPIOA_AFRL |=  (7U << 8);

    // Set baud rate to 9600
    // This assumes default 16 MHz clock
    USART2_BRR = 0x0683;

    // Enable transmitter
    USART2_CR1 |= (1U << 3);

    // Enable USART2
    USART2_CR1 |= (1U << 13);
}

void USART2_SendByte(uint8_t data)
{
    // Wait until transmit register is empty
    while (!(USART2_SR & (1U << 7)))
    {
    }

    // Send the byte
    USART2_DR = data;
}

int main(void)
{
    USART2_Init();

    while (1)
    {
        // Move one direction slowly
        USART2_SendByte(80);
        delay_ms(1000);

        // Stop
        USART2_SendByte(64);
        delay_ms(1000);

        // Move the other direction slowly
        USART2_SendByte(48);
        delay_ms(1000);

        // Stop
        USART2_SendByte(64);
        delay_ms(2000);
    }
}
