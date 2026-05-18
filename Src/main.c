#include <stdint.h>

// Register base addresses for STM32F401RE
#define RCC_BASE        0x40023800UL
#define GPIOA_BASE      0x40020000UL
#define TIM2_BASE       0x40000000UL

// RCC registers
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x40))

// GPIOA registers
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_OTYPER    (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
#define GPIOA_OSPEEDR   (*(volatile uint32_t *)(GPIOA_BASE + 0x08))
#define GPIOA_PUPDR     (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x14))
#define GPIOA_AFRL      (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

// TIM2 registers
#define TIM2_CR1        (*(volatile uint32_t *)(TIM2_BASE + 0x00))
#define TIM2_CCMR1      (*(volatile uint32_t *)(TIM2_BASE + 0x18))
#define TIM2_CCER       (*(volatile uint32_t *)(TIM2_BASE + 0x20))
#define TIM2_PSC        (*(volatile uint32_t *)(TIM2_BASE + 0x28))
#define TIM2_ARR        (*(volatile uint32_t *)(TIM2_BASE + 0x2C))
#define TIM2_CCR1       (*(volatile uint32_t *)(TIM2_BASE + 0x34))
#define TIM2_EGR        (*(volatile uint32_t *)(TIM2_BASE + 0x14))

#define DIR_PIN 6  // PA6 / D12

void delay_ms(volatile uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 800; i++)
    {
        __asm volatile ("nop");
    }
}

void GPIO_DIR_Init(void)
{
    // Turn on GPIOA clock
    RCC_AHB1ENR |= (1U << 0);

    // Set PA6 as output for DIR
    GPIOA_MODER &= ~(3U << (DIR_PIN * 2));
    GPIOA_MODER |=  (1U << (DIR_PIN * 2));

    // Push-pull output
    GPIOA_OTYPER &= ~(1U << DIR_PIN);

    // No pull-up/pull-down
    GPIOA_PUPDR &= ~(3U << (DIR_PIN * 2));
}

void PWM_PA5_Init(void)
{
    // Turn on GPIOA clock
    RCC_AHB1ENR |= (1U << 0);

    // Turn on TIM2 clock
    RCC_APB1ENR |= (1U << 0);

    // Set PA5 to alternate function mode
    // PA5 is D13 on the Nucleo
    GPIOA_MODER &= ~(3U << (5 * 2));
    GPIOA_MODER |=  (2U << (5 * 2));

    // Set PA5 alternate function to AF1, which is TIM2_CH1
    GPIOA_AFRL &= ~(0xFU << (5 * 4));
    GPIOA_AFRL |=  (1U << (5 * 4));

    // Timer setup for PWM
    // Assumes default 16 MHz clock
    // PWM frequency = 16 MHz / (PSC+1) / (ARR+1)
    // Here: 16 MHz / 16 / 1000 = 1000 Hz
    TIM2_PSC = 15;
    TIM2_ARR = 999;

    // PWM mode 1 on channel 1
    TIM2_CCMR1 &= ~(7U << 4);
    TIM2_CCMR1 |=  (6U << 4);

    // Enable preload for channel 1
    TIM2_CCMR1 |= (1U << 3);

    // Enable channel 1 output
    TIM2_CCER |= (1U << 0);

    // Start with motor stopped
    TIM2_CCR1 = 0;

    // Update registers
    TIM2_EGR |= (1U << 0);

    // Enable timer
    TIM2_CR1 |= (1U << 0);
}

void Set_PWM_Percent(uint8_t percent)
{
    if (percent > 100)
    {
        percent = 100;
    }

    TIM2_CCR1 = (percent * 999) / 100;
}

void Set_Direction(uint8_t direction)
{
    if (direction)
    {
        GPIOA_ODR |= (1U << DIR_PIN);   // DIR HIGH
    }
    else
    {
        GPIOA_ODR &= ~(1U << DIR_PIN);  // DIR LOW
    }
}

int main(void)
{
    GPIO_DIR_Init();
    PWM_PA5_Init();

    while (1)
    {
        // Move one direction slowly
        Set_Direction(1);
        Set_PWM_Percent(30);
        delay_ms(1000);

        // Stop
        Set_PWM_Percent(0);
        delay_ms(1000);

        // Move the other direction slowly
        Set_Direction(0);
        Set_PWM_Percent(30);
        delay_ms(1000);

        // Stop
        Set_PWM_Percent(0);
        delay_ms(2000);
    }
}
