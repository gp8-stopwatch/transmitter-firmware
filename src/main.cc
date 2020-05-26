/*
 * GccApplication1.cpp
 *
 * Created: 2019-05-25 10:36:15
 * Author : Dell Latitude E6540
 */

#include <avr/io.h>
#define F_CPU 4000000
#include <avr/interrupt.h>
#include <util/delay.h>

#define ENCODER1_PIN 6
#define ENCODER2_PIN 7

#define SPI_MOSI_PIN 1
#define SPI_MISO_PIN 2
#define SPI_SCK_PIN 3
#define SPI_CS_PIN 0

int main (void)
{
        PORTA.DIR = 0;

        // Init encoder inputs
        PORTA.DIR |= (1 << ENCODER1_PIN | 1 << ENCODER2_PIN);

        // Init SPI
        PORTA.DIR |= (1 << SPI_MOSI_PIN | 1 << SPI_SCK_PIN | 1 << SPI_CS_PIN);

        //        // Enable SPI, Set as Master
        //        // Prescaler: Fosc/16, Enable Interrupts
        //        //The MOSI, SCK pins are as per ATMega8
        //        SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPIE);

        //        SPI0.CTRLA

        //        // Enable Global Interrupts
        //        sei();

        /* Replace with your application code */
        while (1) {
                volatile uint8_t tmp = PORTA.OUT;
                tmp |= 0b11000000;
                PORTA.OUT = tmp;
                _delay_ms (2);

                tmp &= ~0b11000000;
                PORTA.OUT = tmp;
                _delay_ms (2);
        }
}
