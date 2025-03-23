/*
 * calculator.c
 *
 * AVR-GCC version of the calculator.
 * F_CPU is defined for delay routines.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

// =======================================================
// LCD DRIVER FUNCTIONS (using PORTB for LCD)
// Connections (example):
//   PB0: RS, PB1: E, PB2: D4, PB3: D5, PB4: D6, PB5: D7
// =======================================================
#define LCD_RS PB0
#define LCD_E  PB1
#define DAT4   PB2
#define DAT5   PB3
#define DAT6   PB4
#define DAT7   PB5

#define CLEARDISPLAY 0x01

// Bit manipulation macros
#define SetBit(port, bit) ((port) |= _BV(bit))
#define ClearBit(port, bit) ((port) &= ~_BV(bit))

void PulseEnableLine(void) {
    SetBit(PORTB, LCD_E);
    _delay_us(40);
    ClearBit(PORTB, LCD_E);
}

void SendNibble(uint8_t data) {
    // Clear data bits PB2-PB5 (0xC3 = 11000011)
    PORTB &= 0xC3;
    // Set new nibble on bits 2-5
    PORTB |= ((data & 0x0F) << 2);
    PulseEnableLine();
}

void SendByte(uint8_t data) {
    SendNibble(data >> 4);      // Send upper nibble
    SendNibble(data & 0x0F);      // Send lower nibble
}

void LCD_Cmd(uint8_t cmd) {
    ClearBit(PORTB, LCD_RS);  // RS = 0 for command
    SendByte(cmd);
    _delay_us(100);
}

void LCD_Char(uint8_t ch) {
    SetBit(PORTB, LCD_RS);    // RS = 1 for data
    SendByte(ch);
    _delay_us(100);
}

void LCD_Init(void) {
    DDRB = 0xFF;  // Set PORTB as outputs
    _delay_ms(50);
    LCD_Cmd(0x33);
    LCD_Cmd(0x32);
    LCD_Cmd(0x28); // 4-bit mode, 2 lines, 5x7 matrix
    LCD_Cmd(0x0C); // Display ON, cursor OFF
    LCD_Cmd(0x06); // Entry mode: cursor right
    LCD_Cmd(0x01); // Clear display
    _delay_ms(5);
}

void LCD_Clear(void) {
    LCD_Cmd(CLEARDISPLAY);
    _delay_ms(2);
}

void LCD_Message(const char *text) {
    while(*text) {
        LCD_Char(*text++);
    }
}

void LCD_SetCursor(uint8_t col, uint8_t row) {
    LCD_Cmd(0x80 | (col + (row * 0x40)));
}

// =======================================================
// KEYPAD CONFIGURATION
// =======================================================
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 5

// Assume keypad rows are on Port D (PD0-PD3) and columns on Port C (PC0-PC4)
const uint8_t keypad_rows[KEYPAD_ROWS] = { PD0, PD1, PD2, PD3 };
const uint8_t keypad_cols[KEYPAD_COLS] = { PC0, PC1, PC2, PC3, PC4 };

char normalKeys_matrix[KEYPAD_ROWS][KEYPAD_COLS+1] = {
    "123/C",
    "456*D",
    "789-(",
    ".0=+)"
};

char shiftKeys_matrix[KEYPAD_ROWS][KEYPAD_COLS+1] = {
    "sct^C",
    "!Pe|D",  // 'P' stands for "PI"
    "Llbqr",
    "SITQB"
};

volatile uint8_t shiftMode = 0; // Global flag: 0 = off, 1 = on

// =======================================================
// Global Calculator Variables
// =======================================================
#define INPUT_BUFFER_SIZE 50
char inputStr[INPUT_BUFFER_SIZE] = "";
// Using standard C string functions for input management.

// =======================================================
// STACK IMPLEMENTATION
// =======================================================
#define STACK_MAX 20
double valueStack_arr[STACK_MAX];
char operatorStack_arr[STACK_MAX];
int8_t valueTop = -1, operatorTop = -1;

void pushValue_arr(double val) { 
    if(valueTop < STACK_MAX - 1) valueStack_arr[++valueTop] = val; 
}
double popValue_arr(void) { 
    return (valueTop >= 0) ? valueStack_arr[valueTop--] : NAN; 
}
void pushOperator_arr(char op) { 
    if(operatorTop < STACK_MAX - 1) operatorStack_arr[++operatorTop] = op; 
}
char popOperator_arr(void) { 
    return (operatorTop >= 0) ? operatorStack_arr[operatorTop--] : '\0'; 
}
double applyOperator_arr(double a, double b, char op) {
    switch(op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? a / b : NAN;
        case '^': return pow(a, b);
        default: return NAN;
    }
}
int precedence_arr(char op) {
    if(op == '+' || op == '-') return 1;
    if(op == '*' || op == '/') return 2;
    if(op == '^') return 3;
    return 0;
}

// =======================================================
// MATH FUNCTIONS
// =======================================================
double factorial_func(int n) {
    if(n < 0) return NAN;
    double fact = 1;
    for(int i = 1; i <= n; i++) fact *= i;
    return fact;
}

// =======================================================
// HELPER FUNCTIONS
// =======================================================
int startsWith(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

// Evaluate Inverse Trigonometric Functions and basic trig functions
double evaluateTrigFunction(const char *func, double angle_deg) {
    // Normalize angle to [-180, 180]
    while(angle_deg > 180) angle_deg -= 360;
    while(angle_deg < -180) angle_deg += 360;
    int negative = 0;
    if(angle_deg < 0) { negative = 1; angle_deg = -angle_deg; }
    if(angle_deg > 90) angle_deg = 180 - angle_deg;
    if(fabs(angle_deg - 90.0) < 0.001) {
        double result = (strcmp(func, "sin") == 0) ? 1.0 : (strcmp(func, "cos") == 0) ? 0.0 : NAN;
        if ((strcmp(func, "sin") == 0 || strcmp(func, "tan") == 0) && negative)
            result = -result;
        return result;
    }
    double rad = angle_deg * M_PI / 180.0;
    double result;
    if(strcmp(func, "sin") == 0)
         result = sin(rad);
    else if(strcmp(func, "cos") == 0)
         result = cos(rad);
    else if(strcmp(func, "tan") == 0)
         result = tan(rad);
    else
         result = NAN;
    if((strcmp(func, "sin") == 0 || strcmp(func, "tan") == 0) && negative)
         result = -result;
    return result;
}

// =======================================================
// EXPRESSION EVALUATION
// =======================================================
// This is a rudimentary evaluator that uses standard C strings.
// It performs constant replacement and simple function detection.
double evaluateExpression(char *expr) {
    char buffer[100];
    strcpy(buffer, expr);
    char *p;
    // Replace "Pi" with numeric constant.
    while ((p = strstr(buffer, "Pi")) != NULL) {
        char temp[100];
        strcpy(temp, p+2);
        *p = '\0';
        strcat(buffer, "3.141592653589793");
        strcat(buffer, temp);
    }
    // Replace "e" with its constant if not part of a number.
    while ((p = strstr(buffer, "e")) != NULL) {
        if(p == buffer || !isdigit(*(p-1))) {
            char temp[100];
            strcpy(temp, p+1);
            *p = '\0';
            strcat(buffer, "2.718281828459045");
            strcat(buffer, temp);
        } else {
            break;
        }
    }
    
    if(startsWith(buffer, "sin(")) {
        double angle = atof(buffer + 4);
        return evaluateTrigFunction("sin", angle);
    }
    if(startsWith(buffer, "cos(")) {
        double angle = atof(buffer + 4);
        return evaluateTrigFunction("cos", angle);
    }
    if(startsWith(buffer, "tan(")) {
        double angle = atof(buffer + 4);
        return evaluateTrigFunction("tan", angle);
    }
    if(startsWith(buffer, "sininv(")) {
        double val = atof(buffer + 7);
        return asin(val) * (180.0 / M_PI);
    }
    if(startsWith(buffer, "cosinv(")) {
        double val = atof(buffer + 7);
        return acos(val) * (180.0 / M_PI);
    }
    if(startsWith(buffer, "taninv(")) {
        double val = atof(buffer + 7);
        return atan(val) * (180.0 / M_PI);
    }
    if(startsWith(buffer, "ln(")) {
        double val = atof(buffer + 3);
        return log(val);
    }
    if(startsWith(buffer, "log(")) {
        double val = atof(buffer + 4);
        return log10(val);
    }
    if(buffer[strlen(buffer)-1] == '!') {
        int num = atoi(buffer);
        return factorial_func(num);
    }
    // For simplicity, if none of the above, assume it's a simple number.
    return atof(buffer);
}

// =======================================================
// KEY PRESS HANDLING
// =======================================================
void handleKeyPress(char key) {
    // inputStr is our global input buffer.
    if(key == 'C') {
        inputStr[0] = '\0';
    } else if(key == 'D') {
        if(strlen(inputStr) > 0)
            inputStr[strlen(inputStr)-1] = '\0';
    } else if(key == '=') {
        double result = evaluateExpression(inputStr);
        char buf[16];
        dtostrf(result, 6, 3, buf);
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Message(inputStr);
        LCD_SetCursor(0, 1);
        LCD_Message("= ");
        LCD_Message(buf);
        strcpy(inputStr, buf);
    } else {
        // Map key to corresponding string.
        if(key == 's') strcat(inputStr, "sin(");
        else if(key == 'c') strcat(inputStr, "cos(");
        else if(key == 't') strcat(inputStr, "tan(");
        else if(key == 'l') strcat(inputStr, "log(");
        else if(key == 'L') strcat(inputStr, "ln(");
        else if(key == '!') strcat(inputStr, "!");
        else if(key == 'q') strcat(inputStr, "sqrt(");
        else if(key == 'b') strcat(inputStr, "cbrt(");
        else if(key == 'R') strcat(inputStr, "R(");
        else if(key == 'S') strcat(inputStr, "sininv(");
        else if(key == 'I') strcat(inputStr, "cosinv(");
        else if(key == 'T') strcat(inputStr, "taninv(");
        else if(key == '|') strcat(inputStr, "|");
        else if(key == 'Q') strcat(inputStr, "^2");  // Square
        else if(key == 'B') strcat(inputStr, "^3");  // Cube
        else if(key == 'P') strcat(inputStr, "Pi");
        else {
            char temp[2] = {key, '\0'};
            strcat(inputStr, temp);
        }
    }
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Message(inputStr);
}

// =======================================================
// FUNCTION PROTOTYPES
// =======================================================
// Add prototype for scanKeys() so that it is recognized in main()
char scanKeys(void);

// =======================================================
// KEYPAD SCANNING
// =======================================================
const uint8_t keypad_rows_arr[KEYPAD_ROWS] = { PD0, PD1, PD2, PD3 };
const uint8_t keypad_cols_arr[KEYPAD_COLS] = { PC0, PC1, PC2, PC3, PC4 };

char scanKeys(void) {
    for(uint8_t r = 0; r < KEYPAD_ROWS; r++){
        // Drive current row low, others high
        for(uint8_t i = 0; i < KEYPAD_ROWS; i++){
            if(i == r)
                PORTD &= ~_BV(keypad_rows_arr[i]);
            else
                PORTD |= _BV(keypad_rows_arr[i]);
        }
        _delay_ms(5);
        uint8_t cols = ~PINC & 0x1F; // Read PC0-PC4
        if(cols){
            for(uint8_t c = 0; c < KEYPAD_COLS; c++){
                if(cols & _BV(c)){
                    if(shiftMode)
                        return shiftKeys_matrix[r][c];
                    else
                        return normalKeys_matrix[r][c];
                }
            }
        }
    }
    return 0;
}
// =======================================================
// MAIN FUNCTION
// =======================================================
int main(void) {
    // Initialize LCD on PORTB
    DDRB = 0xFF;  // Set all PORTB pins as outputs for LCD
    LCD_Init();
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Message("Calculator Ready");
    _delay_ms(1000);
    LCD_Clear();
    for (int i = 0; i < 10; i++) {
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Counting: ");
    lcd_print_int(i);
    _delay_ms(1000);
}
/*

    // Initialize keypad ports:
    // Rows on Port D (PD0-PD3) as outputs, columns on Port C (PC0-PC4) as inputs with pull-ups.
    DDRD |= 0x0F;         // Lower 4 bits of Port D as outputs
    PORTD |= 0x0F;        // Initially high for rows
    DDRC &= ~0x1F;        // Lower 5 bits of Port C as inputs
    PORTC |= 0x1F;        // Enable pull-ups on PC0-PC4

    // Initialize shift button pin: assume it's on PD7 as input with pull-up.
    DDRD &= ~_BV(PD7);
    PORTD |= _BV(PD7);

    while(1) {
        char key = scanKeys();
        if(key) {
            handleKeyPress(key);
            _delay_ms(200);
        }
        // Update shift mode from PD7
        if(!(PIND & _BV(PD7))) {
            shiftMode = 1;
        } else {
            shiftMode = 0;
        }
        LCD_SetCursor(0, 1);
        if(shiftMode)
            LCD_Message("Shift ON ");
        else
            LCD_Message("         ");
    }
    return 0;*/
}


