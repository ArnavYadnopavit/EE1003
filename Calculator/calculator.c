#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// LCD Setup
#define LCD_DDR DDRB
#define LCD_PORT PORTB
#define LCD_RS 0
#define LCD_E 1
#define DAT4 2
#define DAT5 3
#define DAT6 4
#define DAT7 5

#define CLEARDISPLAY 0x01

// CORDIC Setup
#define ITERATIONS 16
#define SCALE_OUT 32768.0
#define ANGLE_SCALE 16384.0
#define CORDIC_K (0.607252935 * SCALE_OUT)

const int16_t atan_table[ITERATIONS] = {
    8192, 4828, 2552, 1296, 650, 325, 163, 81,
    40, 20, 10, 5, 2, 1, 0, 0
};

// Function prototypes
void LCD_Init(void);
void LCD_Cmd(uint8_t cmd);
void LCD_Char(uint8_t ch);
void LCD_Clear(void);
void LCD_Message(const char *text);
void LCD_Integer(int data);
void cordic(int16_t theta, int16_t *sin_out, int16_t *cos_out);
double evaluateTrigFunction(const char *func, double angle_deg);
double evaluateExpression(const char *expr);
void handleKeyPress(char key);

// Global variables
char input[32] = "";
int input_index = 0;

// Main program
int main(void) {
    // Initialize LCD
    LCD_Init();
    LCD_Clear();
    LCD_Message("Calculator Ready");
    _delay_ms(1000);
    LCD_Clear();

    // Main loop
    while (1) {
        // Simulating key presses for demonstration
        handleKeyPress('2');
        _delay_ms(500);
        handleKeyPress('+');
        _delay_ms(500);
        handleKeyPress('2');
        _delay_ms(500);
        handleKeyPress('=');
        _delay_ms(2000);
        LCD_Clear();
    }

    return 0;
}

// LCD functions implementation
void LCD_Init() {
    LCD_DDR = 0xFF; // Set all pins as output
    _delay_ms(15);
    
    LCD_Cmd(0x33);
    _delay_ms(5);
    LCD_Cmd(0x32);
    _delay_us(100);
    LCD_Cmd(0x28); // 4-bit mode, 2 lines, 5x8 font
    LCD_Cmd(0x0C); // Display on, cursor off, blink off
    LCD_Cmd(0x06); // Increment cursor, no shift
    LCD_Cmd(0x01); // Clear display
    _delay_ms(2);
}

void LCD_Cmd(uint8_t cmd) {
    LCD_PORT &= ~(1 << LCD_RS); // RS = 0 for command
    LCD_PORT = (LCD_PORT & 0x0F) | (cmd & 0xF0);
    LCD_PORT |= (1 << LCD_E);
    _delay_us(1);
    LCD_PORT &= ~(1 << LCD_E);
    _delay_us(100);
    LCD_PORT = (LCD_PORT & 0x0F) | ((cmd << 4) & 0xF0);
    LCD_PORT |= (1 << LCD_E);
    _delay_us(1);
    LCD_PORT &= ~(1 << LCD_E);
    _delay_us(100);
}

void LCD_Char(uint8_t ch) {
    LCD_PORT |= (1 << LCD_RS); // RS = 1 for data
    LCD_PORT = (LCD_PORT & 0x0F) | (ch & 0xF0);
    LCD_PORT |= (1 << LCD_E);
    _delay_us(1);
    LCD_PORT &= ~(1 << LCD_E);
    _delay_us(100);
    LCD_PORT = (LCD_PORT & 0x0F) | ((ch << 4) & 0xF0);
    LCD_PORT |= (1 << LCD_E);
    _delay_us(1);
    LCD_PORT &= ~(1 << LCD_E);
    _delay_us(100);
}

void LCD_Clear() {
    LCD_Cmd(CLEARDISPLAY);
    _delay_ms(2);
}

void LCD_Message(const char *text) {
    while (*text) {
        LCD_Char(*text++);
    }
}

void LCD_Integer(int data) {
    char buffer[16];
    itoa(data, buffer, 10);
    LCD_Message(buffer);
}

// Other functions remain the same

// Key press handling
void handleKeyPress(char key) {
    if (key == 'C') {
        input[0] = '\0';
        input_index = 0;
        LCD_Clear();
    } else if (key == '=') {
        double result = evaluateExpression(input);
        LCD_Clear();
        LCD_Message(input);
        LCD_Cmd(0xC0); // Move to second line
        LCD_Message("= ");
        LCD_Integer((int)result);
        input[0] = '\0';
        input_index = 0;
    } else {
        if (input_index < 31) {
            input[input_index++] = key;
            input[input_index] = '\0';
            LCD_Clear();
            LCD_Message(input);
        }
    }
}

