#define F_CPU 16000000UL  // Add this before delay.h inclusion
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// LCD Configuration
#define LCD_RS 0    // PB0
#define LCD_E  1    // PB1
#define DAT4   2    // PB2
#define DAT5   3    // PB3
#define DAT6   4    // PB4
#define DAT7   5    // PB5

// Button Matrix
#define ROWS 4
#define COLS 5
#define SHIFT_PIN PD7

const uint8_t rowPins[ROWS] = {PD0, PD1, PD2, PD3};
const uint8_t colPins[COLS] = {PC0, PC1, PC2, PC3, PC4};

char normalKeys[ROWS][COLS] = {
  {'1','2','3','/','C'}, {'4','5','6','*','D'},
  {'7','8','9','-','('}, {'.','0','=','+',')'}};

char shiftKeys[ROWS][COLS] = {
  {'s','c','t','^','C'}, {'!','P','e','|','D'},
  {'L','l','b','q','r'}, {'S','I','T','Q','B'}};

// Calculator State
char input[50] = "";
uint8_t input_len = 0;
uint8_t shiftMode = 0;

// Stack Implementation
#define MAX_STACK 20
double valueStack[MAX_STACK];
char operatorStack[MAX_STACK];
int8_t valueTop = -1, operatorTop = -1;

// CORDIC Constants
#define ITERATIONS 16
#define SCALE_OUT 32768.0
#define ANGLE_SCALE 16384.0
#define CORDIC_K (0.607252935 * SCALE_OUT)

const int16_t atan_table[ITERATIONS] = {
  8192,4828,2552,1296,650,325,163,81,40,20,10,5,2,1,0,0};

// Function prototypes
char scanKeys(void);
double evaluateTrigFunction(const char *func, double angle);
void handleKeyPress(char key);
void LCD_SetCursor(uint8_t col, uint8_t row);
double evaluateExpression(char *expr);
double evaluateTrigFunction(const char *func, double angle);
void cordic(int16_t theta, int16_t *sin_out, int16_t *cos_out);
char* strrep(char *orig, char *rep, char *with);
void pushValue(double val);
double popValue();
void pushOperator(char op);
char popOperator();
double applyOperator(double a, double b, char op);
int precedence(char op);

// LCD Functions
void PulseEnable() {
    PORTB |= (1<<LCD_E);
    _delay_us(40);
    PORTB &= ~(1<<LCD_E);
}

void SendNibble(uint8_t data) {
    PORTB = (PORTB & 0xC3) | ((data & 0x0F) << 2);
    PulseEnable();
}

void SendByte(uint8_t data) {
    SendNibble(data >> 4);
    SendNibble(data);
}

void LCD_Cmd(uint8_t cmd) {
    PORTB &= ~(1<<LCD_RS);
    SendByte(cmd);
    _delay_us(100);
}

void LCD_Char(uint8_t ch) {
    PORTB |= (1<<LCD_RS);
    SendByte(ch);
    _delay_us(100);
}

void LCD_Init() {
    DDRB = 0xFF;
    _delay_ms(50);
    LCD_Cmd(0x33);
    LCD_Cmd(0x32);
    LCD_Cmd(0x28);
    LCD_Cmd(0x0C);
    LCD_Cmd(0x06);
    LCD_Cmd(0x01);
    _delay_ms(5);
}

void LCD_Clear() {
    LCD_Cmd(0x01);
    _delay_ms(2);
}

void LCD_Message(const char *text) {
    while(*text) LCD_Char(*text++);
}

void LCD_SetCursor(uint8_t col, uint8_t row) {
    LCD_Cmd(0x80 | (col + (row * 0x40)));
}

// Stack Operations
void pushValue(double val) { 
    if(valueTop < MAX_STACK-1) valueStack[++valueTop] = val; 
}

double popValue() { 
    return (valueTop >= 0) ? valueStack[valueTop--] : NAN; 
}

void pushOperator(char op) { 
    if(operatorTop < MAX_STACK-1) operatorStack[++operatorTop] = op; 
}

char popOperator() { 
    return (operatorTop >= 0) ? operatorStack[operatorTop--] : '\0'; 
}

double applyOperator(double a, double b, char op) {
    switch(op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? a/b : NAN;
        case '^': return pow(a, b);
        default: return NAN;
    }
}

int precedence(char op) {
    if(op == '+' || op == '-') return 1;
    if(op == '*' || op == '/') return 2;
    if(op == '^') return 3;
    return 0;
}

// String replacement function
char* strrep(char *orig, char *rep, char *with) {
    static char buffer[100];
    char *p;
    if(!(p = strstr(orig, rep))) return orig;
    strncpy(buffer, orig, p-orig);
    buffer[p-orig] = '\0';
    sprintf(buffer+(p-orig), "%s%s", with, p+strlen(rep));
    return buffer;
}

// Expression Evaluation
double evaluateExpression(char *expr) {
    // Replace constants
    char *ptr;
    while((ptr = strstr(expr, "Pi"))) {
        memmove(ptr+9, ptr+2, strlen(ptr+2)+1);
        memcpy(ptr, "3.1415926", 9);
    }
    while((ptr = strstr(expr, "e"))) {
        memmove(ptr+12, ptr+1, strlen(ptr+1)+1);
        memcpy(ptr, "2.7182818284", 12);
    }

    // Handle trigonometric functions
    if(strstr(expr, "sin(")) {
        double angle = atof(expr+4);
        return evaluateTrigFunction("sin", angle);
    }
    if(strstr(expr, "cos(")) {
        double angle = atof(expr+4);
        return evaluateTrigFunction("cos", angle);
    }
    if(strstr(expr, "tan(")) {
        double angle = atof(expr+4);
        return evaluateTrigFunction("tan", angle);
    }

    // Handle factorial
    if(expr[strlen(expr)-1] == '!') {
        int num = atoi(expr);
        double fact = 1;
        for(int i=2; i<=num; i++) fact *= i;
        return fact;
    }

    // Basic arithmetic evaluation
    char *endptr;
    double result = strtod(expr, &endptr);
    if(*endptr == '\0') return result;

    return NAN;
}

// Key Handling
void handleKeyPress(char key) {
    if(key == 'C') {
        input[0] = '\0';
        input_len = 0;
        LCD_Clear();
    }
    else if(key == 'D') {
        if(input_len > 0) input[--input_len] = '\0';
    }
    else if(key == '=') {
        double result = evaluateExpression(input);
        char buf[16];
        dtostrf(result, 6, 3, buf);
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Message(input);
        LCD_SetCursor(0, 1);
        LCD_Message("= ");
        LCD_Message(buf);
        strcpy(input, buf);
        input_len = strlen(buf);
    }
    else {
        const char *append = "";
        switch(key) {
            case 's': append = "sin("; break;
            case 'c': append = "cos("; break;
            case 't': append = "tan("; break;
            case 'l': append = "log("; break;
            case 'L': append = "ln("; break;
            case 'q': append = "sqrt("; break;
            case 'b': append = "cbrt("; break;
            case 'R': append = "R("; break;
            case 'S': append = "sininv("; break;
            case 'I': append = "cosinv("; break;
            case 'T': append = "taninv("; break;
            case 'Q': append = "^2"; break;
            case 'B': append = "^3"; break;
            case 'P': append = "Pi"; break;
            default: {
                if(input_len < sizeof(input)-1) {
                    input[input_len++] = key;
                    input[input_len] = '\0';
                }
                return;
            }
        }
        strcat(input, append);
        input_len += strlen(append);
    }
    
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Message(input);
}

int main(void) {
    // Initialize hardware
    DDRD = 0x0F;        // Rows as outputs (PD0-PD3)
    PORTD = 0xF0;       // Enable pull-ups on PD4-PD7
    DDRC = 0x00;        // Columns as inputs
    PORTC = 0x1F;       // Enable pull-ups on PC0-PC4
    
    LCD_Init();
    LCD_Message("Calculator Ready");
    _delay_ms(1000);
    LCD_Clear();

    while(1) {
        char key = scanKeys();
        if(key) handleKeyPress(key);
        
        // Update display
        LCD_SetCursor(0, 0);
        LCD_Message(input);
        LCD_SetCursor(0, 1);
        LCD_Message(shiftMode ? "Shift ON  " : "          ");
    }
}
