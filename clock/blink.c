/*
 * AVR C version of an Arduino clock with a 7447 BCD decoder.
 * 
 * Updated to use per-digit increment logic derived from K-map Boolean expressions.
 *
 * The six digits are:
 *   A B : C D : E F
 *
 * Where:
 *   A = Hours tens (0–2)
 *   B = Hours units (0–9 normally; if A==2 then valid: 0–3)
 *   C = Minutes tens (0–5)
 *   D = Minutes units (0–9)
 *   E = Seconds tens (0–5)
 *   F = Seconds units (0–9)
 *
 * When a digit reaches its maximum value the logic resets it to 0 and 
 * “carries” the increment to the next higher digit.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>

// Global timekeeping variables.
volatile uint32_t timer_millis = 0;  // Incremented by Timer0 ISR

// The six clock digits as described above.
volatile uint8_t A = 0, B = 0, C = 0, D = 0, E = 0, F = 0;

uint32_t lastMillis = 0;

// Button debounce variables.
bool lastHourState = true, lastMinuteState = true;
uint32_t lastHourPress = 0, lastMinutePress = 0;
const uint32_t debounceDelay = 200; // in ms

// Timer0 Compare Match Interrupt Service Routine (generates a 1ms tick)
ISR(TIMER0_COMPA_vect) {
    timer_millis++;
}

// Returns the number of milliseconds since the timer started (atomic read).
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

/*
 * Displays one digit on the 7447 using multiplexing.
 * "digit" is the value 0-9 and "position" selects which of the 6 digits to enable.
 */
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
        case 0: PORTD |= (1 << PD6); break;   // Digit 0 (A) on PD6.
        case 1: PORTD |= (1 << PD7); break;   // Digit 1 (B) on PD7.
        case 2: PORTB |= (1 << PB0); break;   // Digit 2 (C) on PB0.
        case 3: PORTB |= (1 << PB1); break;   // Digit 3 (D) on PB1.
        case 4: PORTB |= (1 << PB2); break;   // Digit 4 (E) on PB2.
        case 5: PORTB |= (1 << PB3); break;   // Digit 5 (F) on PB3.
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

/*
 * Updates the display by cycling through all 6 digits:
 * A B : C D : E F
 */
void updateDisplay(void) {
    displayDigit(A, 0);
    displayDigit(B, 1);
    displayDigit(C, 2);
    displayDigit(D, 3);
    displayDigit(E, 4);
    displayDigit(F, 5);
}

/*
 * Update time using per-digit increment logic:
 *
 * Seconds: 
 *    - Increment seconds units (F) mod-10.
 *    - When F > 9, reset F = 0 and increment seconds tens (E) mod-6.
 *
 * Minutes:
 *    - When seconds roll over, increment minutes units (D) mod-10.
 *    - When D > 9, reset D = 0 and increment minutes tens (C) mod-6.
 *
 * Hours:
 *    - When minutes roll over, increment hours units (B).
 *    - For A < 2, B is a mod-10 counter.
 *    - For A == 2, only 0–3 are allowed for B (i.e. valid hours 20–23).
 *    - A itself is a mod-3 counter.
 */
void updateTime(void) {
    // Increment seconds units (F) – mod-10 counter.
    F++;
    if (F > 9) {
        F = 0;
        // Increment seconds tens (E) – mod-6 counter.
        E++;
        if (E > 5) {
            E = 0;
            // Seconds have fully rolled over, so increment minutes.
            // Increment minutes units (D) – mod-10 counter.
            D++;
            if (D > 9) {
                D = 0;
                // Increment minutes tens (C) – mod-6 counter.
                C++;
                if (C > 5) {
                    C = 0;
                    // Minutes have fully rolled over; increment hours.
                    B++;  // Increment hours units.
                    if (A < 2) {  // For hours 0-19
                        if (B > 9) {
                            B = 0;
                            A++;  // Increment hours tens.
                            if (A > 2) { // safeguard (should not occur)
                                A = 0;
                            }
                        }
                    } else {  // A == 2, valid hours are 20–23.
                        if (B > 3) {
                            B = 0;
                            A = 0;  // roll over from 23:.. to 00:..
                        }
                    }
                }
            }
        }
    }
}

/*
 * Helper functions to convert between the two-digit representation
 * and the full hour or minute value.
 */
uint8_t getHours(void) {
    return A * 10 + B;
}
void setHours(uint8_t h) {
    A = h / 10;
    B = h % 10;
}
uint8_t getMinutes(void) {
    return C * 10 + D;
}
void setMinutes(uint8_t m) {
    C = m / 10;
    D = m % 10;
}

/*
 * Reads the button states, performs debouncing,
 * and increments hours or minutes as needed.
 * When a button is pressed, the full hour or minute value is updated,
 * then the digit variables (A/B or C/D) are re–computed.
 */
void checkButtons(void) {
    // Read button states: when not pressed, the pull-up makes these HIGH.
    bool hourState = (PINB & (1 << PB4)) != 0;    // true when not pressed.
    bool minuteState = (PINB & (1 << PB5)) != 0;
    uint32_t currentMillis = millis();
    
    // Hour button (active low)
    if (!hourState && lastHourState && ((currentMillis - lastHourPress) > debounceDelay)) {
        uint8_t h = getHours();
        h = (h + 1) % 24;
        setHours(h);
        lastHourPress = currentMillis;
    }
    
    // Minute button (active low)
    if (!minuteState && lastMinuteState && ((currentMillis - lastMinutePress) > debounceDelay)) {
        uint8_t m = getMinutes();
        m = (m + 1) % 60;
        setMinutes(m);
        lastMinutePress = currentMillis;
    }
    
    lastHourState = hourState;
    lastMinuteState = minuteState;
}

/*
 * Main loop:
 *  - Every 1000 ms, update the time by calling updateTime() (which cascades the increments).
 *  - Check buttons and refresh the multiplexed display.
 */
int main(void) {
    setup();
    sei();  // Enable global interrupts.
    
    while (1) {
        uint32_t currentMillis = millis();
        if ((currentMillis - lastMillis) >= 1000) {
            lastMillis = currentMillis;
            updateTime();
        }
        
        checkButtons();
        updateDisplay();
    }
    
    return 0;
}

