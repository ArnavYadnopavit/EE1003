#include <avr/io.h> 
#include <util/delay.h> 
#include <stdlib.h> 
#include <math.h>

// ---------------- TYPEDEFS ----------------
typedef uint8_t byte; 

// ---------------- LCD DRIVER ROUTINES ----------------
#define ClearBit(x,y) x &= ~_BV(y) 
#define SetBit(x,y) x |= _BV(y) 

// LCD connection pins (Port B)
#define LCD_RS 0  
#define LCD_E 1   
#define DAT4 2    
#define DAT5 3    
#define DAT6 4    
#define DAT7 5    
#define CLEARDISPLAY 0x01

void PulseEnableLine() {
    SetBit(PORTB, LCD_E);
    _delay_us(40);
    ClearBit(PORTB, LCD_E);
}

void SendNibble(byte data) {
    PORTB &= 0xC3;  
    if (data & _BV(4)) SetBit(PORTB, DAT4);
    if (data & _BV(5)) SetBit(PORTB, DAT5);
    if (data & _BV(6)) SetBit(PORTB, DAT6);
    if (data & _BV(7)) SetBit(PORTB, DAT7);
    PulseEnableLine();
}

void SendByte(byte data) {
    SendNibble(data);
    SendNibble(data << 4);
    ClearBit(PORTB, 5);
}

void LCD_Cmd(byte cmd) {
    ClearBit(PORTB, LCD_RS);
    SendByte(cmd);
}

void LCD_Char(byte ch) {
    SetBit(PORTB, LCD_RS);
    SendByte(ch);
}

void LCD_Init() {
    LCD_Cmd(0x33);
    LCD_Cmd(0x32);
    LCD_Cmd(0x28);
    LCD_Cmd(0x0C);
    LCD_Cmd(0x06);
    LCD_Cmd(0x01);
    _delay_ms(3);
}

void LCD_Clear() {
    LCD_Cmd(CLEARDISPLAY);
    _delay_ms(3);
}

void LCD_Message(const char *text) {
    while (*text) LCD_Char(*text++);
}

void LCD_Integer(int data) {
    char st[8] = "";
    itoa(data, st, 10);
    LCD_Message(st);
}

// ---------------- Button Matrix Setup ----------------
#define ROWS 4
#define COLS 5
#define SHIFT_BUTTON 13  

int rowPins[ROWS] = {2, 3, 4, 5};
int colPins[COLS] = {6, 7, 8, 9, 10};

char normalKeys[ROWS][COLS] = {
    {'1', '2', '3', '/', 'C'},
    {'4', '5', '6', '*', 'D'},
    {'7', '8', '9', '-', '('},
    {'.', '0', '=', '+', ')'}
};

char shiftKeys[ROWS][COLS] = {
    {'s', 'c', 't', '^', 'C'},
    {'!', 'P', 'e', '|', 'D'},
    {'L', 'l', 'b', 'q', 'r'},
    {'S', 'I', 'T', 'Q', 'B'}
};

String input = "";

// ---------------- Stack Implementation ----------------
#define MAX_STACK 20
double valueStack[MAX_STACK];
char operatorStack[MAX_STACK];
int valueTop = -1, operatorTop = -1;

void pushValue(double val) { 
    if (valueTop < MAX_STACK - 1) valueStack[++valueTop] = val; 
}
double popValue() { 
    return (valueTop >= 0) ? valueStack[valueTop--] : 0; 
}

void pushOperator(char op) { 
    if (operatorTop < MAX_STACK - 1) operatorStack[++operatorTop] = op; 
}
char popOperator() { 
    return (operatorTop >= 0) ? operatorStack[operatorTop--] : '\0'; 
}

double applyOperator(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? (a / b) : NAN;
        default: return 0;
    }
}

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}
// ---------------- CORDIC Setup ----------------
#define ITERATIONS 16
#define SCALE_OUT 32768.0      
#define ANGLE_SCALE 16384.0    
#define CORDIC_K (0.607252935 * SCALE_OUT)

const int16_t atan_table[ITERATIONS] = {
    8192, 4828, 2552, 1296, 650, 325, 163, 81,
    40, 20, 10, 5, 2, 1, 0, 0
};

void cordic(int16_t theta, int16_t *sin_out, int16_t *cos_out) {
    int32_t x = CORDIC_K, y = 0, z = theta;
    for (int i = 0; i < ITERATIONS; i++) {
        int32_t x_new, y_new;
        if (z >= 0) {
            x_new = x - (y >> i);
            y_new = y + (x >> i);
            z -= atan_table[i];
        } else {
            x_new = x + (y >> i);
            y_new = y - (x >> i);
            z += atan_table[i];
        }
        x = x_new;
        y = y_new;
    }
    *cos_out = (int16_t)x;
    *sin_out = (int16_t)y;
}

// ---------------- Math Functions ----------------
double factorial(int n) {
    if (n < 0) return NAN;
    double fact = 1;
    for (int i = 1; i <= n; i++) fact *= i;
    return fact;
}

double rk4_ln(double x0, double x) {
    if (x0 <= 0 || x <= 0) return NAN;
    int n = 100;
    double h = (x - x0) / n;
    double y = log(x0);

    for (int i = 0; i < n; i++) {
        double k1 = h / x0;
        double k2 = h / (x0 + h / 2);
        double k3 = h / (x0 + h / 2);
        double k4 = h / (x0 + h);
        y += (k1 + 2 * k2 + 2 * k3 + k4) / 6;
        x0 += h;
    }
    return y;
}

double rk4_sqrt(double x, double h = 0.01, int steps = 100) {
    double y = 1;
    double xn = x;

    for (int i = 0; i < steps; i++) {
        double k1 = 1 / (2 * sqrt(xn));
        double k2 = 1 / (2 * sqrt(xn + h / 2));
        double k3 = 1 / (2 * sqrt(xn + h / 2));
        double k4 = 1 / (2 * sqrt(xn + h));
        y += h / 6 * (k1 + 2 * k2 + 2 * k3 + k4);
        xn += h;
    }
    return y;
}

double rk4_asin(double x, double h = 0.01, int steps = 100) {
    double y = 0;
    double xn = 0;

    for (int i = 0; i < steps && xn < x; i++) {
        double k1 = 1.0 / sqrt(1 - xn * xn);
        double k2 = 1.0 / sqrt(1 - (xn + h / 2) * (xn + h / 2));
        double k3 = 1.0 / sqrt(1 - (xn + h / 2) * (xn + h / 2));
        double k4 = 1.0 / sqrt(1 - (xn + h) * (xn + h));

        y += h / 6 * (k1 + 2 * k2 + 2 * k3 + k4);
        xn += h;
    }
    return y;
}

double rk4_acos(double x) {
    return M_PI / 2 - rk4_asin(x);
}

double rk4_atan(double x, double h = 0.01, int steps = 100) {
    double y = 0;
    double xn = 0;

    for (int i = 0; i < steps && xn < x; i++) {
        double k1 = 1.0 / (1 + xn * xn);
        double k2 = 1.0 / (1 + (xn + h / 2) * (xn + h / 2));
        double k3 = 1.0 / (1 + (xn + h / 2) * (xn + h / 2));
        double k4 = 1.0 / (1 + (xn + h) * (xn + h));

        y += h / 6 * (k1 + 2 * k2 + 2 * k3 + k4);
        xn += h;
    }
    return y;
}
// ---------------- Expression Evaluation ----------------
double evaluateExpression(char* expr) {
    valueTop = -1;
    operatorTop = -1;

    int i = 0;
    while (expr[i] != '\0') {
        if ((expr[i] >= '0' && expr[i] <= '9') || expr[i] == '.') {
            char numStr[10] = "";
            int j = 0;
            while ((expr[i] >= '0' && expr[i] <= '9') || expr[i] == '.') {
                numStr[j++] = expr[i++];
            }
            numStr[j] = '\0';
            pushValue(atof(numStr));
            i--;
        } else if (expr[i] == '(') {
            pushOperator(expr[i]);
        } else if (expr[i] == ')') {
            while (operatorTop >= 0 && operatorStack[operatorTop] != '(') {
                double b = popValue();
                double a = popValue();
                char op = popOperator();
                pushValue(applyOperator(a, b, op));
            }
            popOperator();
        } else if (isalpha(expr[i])) {  // Detect function names
            char funcName[10] = "";
            int j = 0;
            while (isalpha(expr[i])) {
                funcName[j++] = expr[i++];
            }
            funcName[j] = '\0';
            if (expr[i] == '(') {
                i++;
                double param = evaluateExpression(expr + i);
                while (expr[i] != ')') i++;
                if (strcmp(funcName, "sin") == 0) {
                    int16_t sinVal, cosVal;
                    cordic(param * ANGLE_SCALE / M_PI_2, &sinVal, &cosVal);
                    pushValue(sinVal / SCALE_OUT);
                } else if (strcmp(funcName, "cos") == 0) {
                    int16_t sinVal, cosVal;
                    cordic(param * ANGLE_SCALE / M_PI_2, &sinVal, &cosVal);
                    pushValue(cosVal / SCALE_OUT);
                } else if (strcmp(funcName, "tan") == 0) {
                    int16_t sinVal, cosVal;
                    cordic(param * ANGLE_SCALE / M_PI_2, &sinVal, &cosVal);
                    pushValue((cosVal != 0) ? (sinVal / (double)cosVal) : NAN);
                } else if (strcmp(funcName, "ln") == 0) {
                    pushValue(rk4_ln(1, param));
                } else if (strcmp(funcName, "sqrt") == 0) {
                    pushValue(rk4_sqrt(param));
                } else if (strcmp(funcName, "asin") == 0) {
                    pushValue(rk4_asin(param));
                } else if (strcmp(funcName, "acos") == 0) {
                    pushValue(rk4_acos(param));
                } else if (strcmp(funcName, "atan") == 0) {
                    pushValue(rk4_atan(param));
                }
            }
        } else { 
            while (operatorTop >= 0 && precedence(operatorStack[operatorTop]) >= precedence(expr[i])) {
                double b = popValue();
                double a = popValue();
                char op = popOperator();
                pushValue(applyOperator(a, b, op));
            }
            pushOperator(expr[i]);
        }
        i++;
    }

    while (operatorTop >= 0) {
        double b = popValue();
        double a = popValue();
        char op = popOperator();
        pushValue(applyOperator(a, b, op));
    }

    return popValue();
}


// ---------------- Key Press Handling ----------------
void handleKeyPress(char key) {
    lcd_set_cursor(0, 0);

    if (key == 'C') {
        input_len = 0;
        input[0] = '\0';
        lcd_clear();
    } else if (key == 'D') {
        if (input_len > 0) {
            input[--input_len] = '\0';
        }
    } else if (key == '=') {
        double result = evaluateExpression(input);
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print(input);
        lcd_set_cursor(0, 1);
        char resultStr[16];
        dtostrf(result, 6, 2, resultStr);
        lcd_print("= ");
        lcd_print(resultStr);
        snprintf(input, sizeof(input), "%s", resultStr);
        input_len = strlen(input);
    } else {
        if (input_len < sizeof(input) - 1) {
            if (key == 's') strcat(input, "sin(");
            else if (key == 'c') strcat(input, "cos(");
            else if (key == 't') strcat(input, "tan(");
            else if (key == 'l') strcat(input, "log(");
            else if (key == 'L') strcat(input, "ln(");
            else if (key == '!') strcat(input, "!");
            else if (key == 'q') strcat(input, "sqrt(");
            else if (key == 'b') strcat(input, "cbrt(");  // Cube root (not implemented yet)
            else if (key == 'N') strcat(input, "N");
            else if (key == 'E') strcat(input, "E");
            else if (key == 'R') strcat(input, "R(");
            else if (key == 'S') strcat(input, "sininv(");
            else if (key == 'I') strcat(input, "cosinv(");
            else if (key == 'T') strcat(input, "taninv(");
            else if (key == '|') strcat(input, "|");
            else if (key == 'Q') strcat(input, "^2");  // Square
            else if (key == 'B') strcat(input, "^3");  // Cube
            else if (key == 'P') strcat(input, "Pi");
            else {
                input[input_len++] = key;
                input[input_len] = '\0';
            }
        }
    }

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print(input);
}


// ---------------- Setup ----------------
void setup() {
    LCD_Init();
    LCD_Message("Calculator Ready");

    DDRC = 0x0F;  
    PORTC = 0xF0; 

    DDRD &= ~(1 << SHIFT_BUTTON); 
    PORTD |= (1 << SHIFT_BUTTON); 
}

// ---------------- Main Loop ----------------
void loop() {
    uint8_t lastShiftState = PIND & (1 << SHIFT_BUTTON);
    _delay_ms(10);
    uint8_t currentShiftState = PIND & (1 << SHIFT_BUTTON);

    static bool shiftMode = false;

    if (lastShiftState == 1 && currentShiftState == 0) {
        shiftMode = !shiftMode;
        _delay_ms(200);
    }

    LCD_Clear();
    LCD_Message(shiftMode ? "Shift ON " : "         ");

    for (uint8_t i = 0; i < ROWS; i++) {
        PORTC = ~(1 << i);
        _delay_ms(5);

        for (uint8_t j = 0; j < COLS; j++) {
            if (!(PINC & (1 << (j + 4)))) {
                _delay_ms(50);
                char key = shiftMode ? shiftKeys[i][j] : normalKeys[i][j];
                handleKeyPress(key);
                while (!(PINC & (1 << (j + 4))));
                _delay_ms(50);
            }
        }
    }
}

