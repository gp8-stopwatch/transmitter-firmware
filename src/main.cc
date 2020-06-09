/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <string.h>
#include <util/delay.h>

static constexpr int BATTERY_FULL = 42;     // 4.2
static constexpr int BATTERY_LOW = 33;      // 3.3
static constexpr int BATTERY_CRITICAL = 30; // 3.0

enum Pin { redLed = 0, greenLed = 1, senseOn = 2, ir = 6, batSense = 7 };

static constexpr long int CLK_PER_Hz = 3300000;
static constexpr long int CARRIER_Hz = 56000;
static constexpr uint8_t PWM_CNT_VALUE = uint8_t ((CLK_PER_Hz / CARRIER_Hz) + 0.5F);

void reverse (char s[])
{
        int i, j;
        char c;

        for (i = 0, j = strlen (s) - 1; i < j; i++, j--) {
                c = s[i];
                s[i] = s[j];
                s[j] = c;
        }
}

void itoa (unsigned int n, char s[])
{
        int i;

        i = 0;

        do {                           /* generate digits in reverse order */
                s[i++] = n % 10 + '0'; /* get next digit */
        } while ((n /= 10) > 0);       /* delete it */

        s[i] = '\0';

        reverse (s);
}

void print (uint8_t const *data, int len)
{
        for (int i = 0; i < len; ++i) {

                // Wait until writing to the register is allowed
                while ((USART0.STATUS & USART_DREIF_bm) == 0) {
                }

                USART0.TXDATAL = *(data + i);

                // Wait until sent. Not strictly necessary I think.
                while ((USART0.STATUS & USART_TXCIF_bm) == 0) {
                }
        }
}

void print (const char *str) { print (reinterpret_cast<const uint8_t *> (str), strlen (str)); }

void print (unsigned int i)
{
        char buf[11];
        itoa (i, buf);
        print (buf);
}

/**
 * Main battery voltage sensing.
 */
int getBatteryVoltage ()
{
        // Turn the voltage divider on.
        PORTA.OUT |= (1 << Pin::senseOn);

        // Start the ADC conversion
        ADC0.COMMAND = ADC_STCONV_bm; // This starts the measurement

        // Wait for the converison to finish
        while ((ADC0.COMMAND & ADC_STCONV_bm) != 0 && (ADC0.INTFLAGS & ADC_RESRDY_bm) == 0) {
        }

        // Turn the voltage divider off.
        PORTA.OUT &= ~(1 << Pin::senseOn);

        return ADC0.RES / 163;
}

void toggleGreen ()
{
        static bool b{};

        if (b) {
                PORTA.OUT |= (1 << Pin::greenLed);
        }
        else {
                PORTA.OUT &= ~(1 << Pin::greenLed);
        }

        b = !b;
}

void toggleRed ()
{
        static bool b{};

        if (b) {
                PORTA.OUT |= (1 << Pin::redLed);
        }
        else {
                PORTA.OUT &= ~(1 << Pin::redLed);
        }

        b = !b;
}

template <typename T, T low, T hi, typename Fn> struct Hysteresis {

        Hysteresis (Fn fn) : fn{/* std::move */ (fn)} {}

        bool operator() ()
        {
                if (fn () <= low) {
                        isLow = true;
                }
                else if (fn () >= hi) {
                        isLow = false;
                }

                return isLow;
        }

        operator bool () const { return isLow; }

        Fn fn;
        bool isLow{};
};

/**
 * CPU is running @ 20MHz, so CLK_PER is 3.3MHz
 */
int main ()
{
        // Configure output pins. Rest is left as they were, i.e. inputs.
        PORTA.DIR = (1 << Pin::ir) | (1 << Pin::senseOn) | (1 << Pin::redLed) | (1 << Pin::greenLed);

        // All output pins to off.
        PORTA.OUT = 0x00;

        /*--------------------------------------------------------------------------*/
        // UART for debugging.
#ifdef WITH_DEBUG
        volatile uint8_t tmp = PORTA.OUT;
        tmp |= (1 << Pin::ir);
        PORTA.OUT = tmp;               // High
        USART0.BAUD = 115;             // 115200
        USART0.CTRLB |= USART_TXEN_bm; // Enable transmitter
#else
        /*--------------------------------------------------------------------------*/
        // PWM for the IR.

        // Load CCMP register with the period and duty cycle of the PWM
        TCB0.CCMPL = PWM_CNT_VALUE;
        TCB0.CCMPH = PWM_CNT_VALUE / 2;
        TCB0.CNT = 0;

        // Enable TCB clock == CLK_PER
        TCB0.CTRLA |= TCB_ENABLE_bm;

        // Enable Pin Output and configure TCB in 8-bit PWM mode
        TCB0.CTRLB |= TCB_CCMPEN_bm;
        TCB0.CTRLB |= TCB_CNTMODE_PWM8_gc;
#endif
        /*--------------------------------------------------------------------------*/
        // ADC

        VREF.CTRLA |= VREF_ADC0REFSEL_2V5_gc;              // Voltage reference is set to 2.5V, and divider divides +BATT by 2.
        ADC0.CTRLB |= (ADC_SAMPNUM0_bm | ADC_SAMPNUM1_bm); // Accumulate 8 consecutive results
        ADC0.CTRLC |= (ADC_PRESC0_bm | ADC_PRESC1_bm | ADC_PRESC2_bm) /* | ADC_REFSEL0_bm */; // Prescaler set to divide by 256 (slow)
        ADC0.MUXPOS = 0x07;                                                                   // measure on PA7
        // ADC0.MUXPOS = 0x00;
        ADC0.CTRLA |= ADC_ENABLE_bm; // Enable in 10bits resolution. Use internal voltage reference

        /*--------------------------------------------------------------------------*/

        auto a = [] { return getBatteryVoltage (); };
        Hysteresis<int, 33, 35, decltype (a)> lowVoltage (a);

        while (true) {
                // volatile uint8_t tmp = PORTA.OUT;
                // tmp |= (1 << Pin::ir);
                // PORTA.OUT = tmp;
                // _delay_ms (1);

                // tmp &= ~(1 << Pin::ir);
                // PORTA.OUT = tmp;
                _delay_ms (50);

                auto adcResult = getBatteryVoltage ();

                if (adcResult < BATTERY_CRITICAL) {
                        print ("C ");
                        PORTA.OUT = 0x00;
                        PORTA.DIR = 0; // all pins to inputs
                        set_sleep_mode (SLEEP_MODE_PWR_DOWN);
                        sleep_enable ();
                        sleep_cpu ();
                }
                else if (lowVoltage ()) {
                        toggleRed ();
                        PORTA.OUT &= ~(1 << Pin::greenLed);
                        print ("R ");
                }
                else {
                        toggleGreen ();
                        PORTA.OUT &= ~(1 << Pin::redLed);
                        print ("G ");
                }

#ifdef WITH_DEBUG
                print (adcResult);
                print ("\r\n");
#endif
        }
}
