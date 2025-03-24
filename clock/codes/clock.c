/*
 * AVR C version of an Arduino clock with a 7447 BCD decoder.
 * This code configures:
 *  - PD2-PD5 for the 4 BCD output bits (driving a 7447)
 *  - PD6, PD7 and PB0-PB3 for digit enable (multiplexing 6 digits)
 *  - PB4, PB5 as inputs with pull-ups for hour and minute buttons.
 *
 * Timer0 is used to generate a 1 ms tick (similar to Arduino millis()).
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>

// Global timekeeping variables.
volatile uint32_t timer_millis = 0;  // Incremented by Timer0 ISR
uint8_t hours = 0, minutes = 0, seconds = 0;
uint32_t lastMillis = 0;

// Button debounce variables.
bool lastHourState = true, lastMinuteState = true;
uint32_t lastHourPress = 0, lastMinutePress = 0;
const uint32_t debounceDelay = 200; // in ms

// Timer0 Compare Match Interrupt Service Routine
ISR(TIMER0_COMPA_vect) {
    timer_millis++;
}

// Returns the number of milliseconds since the timer started.
// (Atomic read of the 32-bit variable.)
uint32_t millis(void) {
    uint32_t ms;
    cli();
    ms = timer_millis;
    sei();
    return ms;
}

void setup(void) {
    // --- Configure BCD segment output pins (PD2-PD5) as outputs ---
    // Also configure PD6 and PD7 for the first two digit enable signals.
    DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
    
    // --- Configure digit enable pins on PORTB (PB0-PB3) as outputs ---
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
    
    // --- Configure buttons (hour & minute) on PB4 and PB5 as inputs with pull-ups ---
    DDRB &= ~((1 << PB4) | (1 << PB5));     // Set PB4 and PB5 as inputs.
    PORTB |= (1 << PB4) | (1 << PB5);         // Enable internal pull-up resistors.
    
    // --- Initialize output pins to low ---
    PORTD &= ~((1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5)); // Clear BCD pins.
    PORTD &= ~((1 << PD6) | (1 << PD7));                           // Clear digit enable (PD6, PD7).
    PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3));   // Clear digit enable (PB0-PB3).
    
    // --- Set up Timer0 to generate an interrupt every 1 ms ---
    TCCR0A = (1 << WGM01);                     // Configure Timer0 in CTC mode.
    TCCR0B = (1 << CS01) | (1 << CS00);          // Prescaler 64 (16MHz/64 = 250kHz).
    OCR0A = 249;                               // (250kHz/1000) - 1 = 249 --> 1ms period.
    TIMSK0 |= (1 << OCIE0A);                   // Enable Timer0 Compare Match A interrupt.
}

//
// Displays one digit on the 7447 using multiplexing.
// "digit" is the value 0-9 and "position" selects which of the 6 digits to enable.
//
void displayDigit(uint8_t digit, uint8_t position) {
    // Set the BCD output bits on PD2-PD5.
    if (digit & 0x01)
        PORTD |= (1 << PD2);
    else
        PORTD &= ~(1 << PD2);
    
    if (digit & 0x02)
        PORTD |= (1 << PD3);
    else
        PORTD &= ~(1 << PD3);
    
    if (digit & 0x04)
        PORTD |= (1 << PD4);
    else
        PORTD &= ~(1 << PD4);
    
    if (digit & 0x08)
        PORTD |= (1 << PD5);
    else
        PORTD &= ~(1 << PD5);
    
    // Turn off all digit enable pins.
    PORTD &= ~((1 << PD6) | (1 << PD7));                         // For digits 0 and 1.
    PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3)); // For digits 2-5.
    
    // Enable the specific digit based on its position.
    switch(position) {
        case 0: PORTD |= (1 << PD6); break;   // Digit 0 on PD6.
        case 1: PORTD |= (1 << PD7); break;   // Digit 1 on PD7.
        case 2: PORTB |= (1 << PB0); break;   // Digit 2 on PB0.
        case 3: PORTB |= (1 << PB1); break;   // Digit 3 on PB1.
        case 4: PORTB |= (1 << PB2); break;   // Digit 4 on PB2.
        case 5: PORTB |= (1 << PB3); break;   // Digit 5 on PB3.
        default: break;
    }
    
    // Small delay to let the digit be visible (reduces flicker).
    _delay_ms(3);
    
    // Disable the digit after the delay.
    switch(position) {
        case 0: PORTD &= ~(1 << PD6); break;
        case 1: PORTD &= ~(1 << PD7); break;
        case 2: PORTB &= ~(1 << PB0); break;
        case 3: PORTB &= ~(1 << PB1); break;
        case 4: PORTB &= ~(1 << PB2); break;
        case 5: PORTB &= ~(1 << PB3); break;
        default: break;
    }
}

//
// Updates the display by cycling through all 6 digits:
// Hours (two digits), minutes (two digits), seconds (two digits).
//
void updateDisplay(void) {
    displayDigit(hours / 10, 0);
    displayDigit(hours % 10, 1);
    displayDigit(minutes / 10, 2);
    displayDigit(minutes % 10, 3);
    displayDigit(seconds / 10, 4);
    displayDigit(seconds % 10, 5);
}

//
// Reads the button states, performs debouncing,
// and increments hours or minutes as needed.
//
void checkButtons(void) {
    // Read button states: when not pressed, the pull-up makes these HIGH.
    bool hourState = (PINB & (1 << PB4)) != 0;    // true when not pressed.
    bool minuteState = (PINB & (1 << PB5)) != 0;
    uint32_t currentMillis = millis();
    
    // If the hour button is pressed (active low) and it just changed state, update the hour.
    if (!hourState && lastHourState && ((currentMillis - lastHourPress) > debounceDelay)) {
        hours = (hours + 1) % 24;
        lastHourPress = currentMillis;
    }
    
    // If the minute button is pressed and it just changed state, update the minute.
    if (!minuteState && lastMinuteState && ((currentMillis - lastMinutePress) > debounceDelay)) {
        minutes = (minutes + 1) % 60;
        lastMinutePress = currentMillis;
    }
    
    lastHourState = hourState;
    lastMinuteState = minuteState;
}

//
// Main loop: update time once every second, check the buttons,
// and continually refresh the multiplexed display.
//
int main(void) {
    setup();
    sei();  // Enable global interrupts.
    
    while (1) {
        uint32_t currentMillis = millis();
        if ((currentMillis - lastMillis) >= 1000) {
            lastMillis = currentMillis;
            seconds++;
            if (seconds >= 60) {
                seconds = 0;
                minutes++;
            }
            if (minutes >= 60) {
                minutes = 0;
                hours++;
            }
            if (hours >= 24) {
                hours = 0;
            }
        }
        
        checkButtons();
        updateDisplay();
    }
    
    return 0;
}

