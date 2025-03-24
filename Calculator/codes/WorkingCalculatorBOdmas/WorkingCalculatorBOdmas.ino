#include <LiquidCrystal.h>
#include <math.h>

// LCD setup (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

// Button matrix setup
#define ROWS 4
#define COLS 5
#define SHIFT_BUTTON 13  // Shift button

// Row and Column Pins
int rowPins[ROWS] = {2, 3, 4, 5};  
int colPins[COLS] = {6, 7, 8, 9, 10};  

// Normal and Shift Key Mappings
char normalKeys[ROWS][COLS] = {
    {'1', '2', '3', '/', 'C'},
    {'4', '5', '6', '*', 'D'},
    {'7', '8', '9', '-', '('},
    {'.', '0', '=', '+', ')'}
};

char shiftKeys[ROWS][COLS] = {
    {'s', 'c', 't', '^', 'R'}, 
    {'(', ')', 'π', '!', 'E'}, 
    {'%', 'l', 'L', 'e', 'D'}, 
    {'A', 'B', 'C', 'D', 'M'}
};

// Input buffer
String input = "";

// Stack Implementation
#define MAX_STACK 20
double valueStack[MAX_STACK];
char operatorStack[MAX_STACK];
int valueTop = -1, operatorTop = -1;

// Stack functions
void pushValue(double val) { if (valueTop < MAX_STACK - 1) valueStack[++valueTop] = val; }
double popValue() { return (valueTop >= 0) ? valueStack[valueTop--] : 0; }

void pushOperator(char op) { if (operatorTop < MAX_STACK - 1) operatorStack[++operatorTop] = op; }
char popOperator() { return (operatorTop >= 0) ? operatorStack[operatorTop--] : '\0'; }

// Function to apply an operation
double applyOperator(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? (a / b) : NAN;
        case '^': return pow(a, b);
        default: return 0;
    }
}

// Function to return precedence of an operator
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}

// Function to evaluate expression
double evaluateExpression(String expr) {
    expr.replace("π", "3.141592653589793");
    expr.replace("e", "2.718281828459045");

    valueTop = -1;
    operatorTop = -1;

    int i = 0;
    while (i < expr.length()) {
        if (isdigit(expr[i]) || expr[i] == '.') {
            String numStr = "";
            while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) {
                numStr += expr[i++];
            }
            pushValue(numStr.toFloat());
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
            popOperator();  // Remove '('
        } else {  // Operator found
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

// Function to handle key press
void handleKeyPress(char key) {
    lcd.setCursor(0, 0);

    if (key == 'C') {  
        input = "";
        lcd.clear();
    } 
    else if (key == 'D') {  
        if (input.length() > 0) {
            input.remove(input.length() - 1);
        }
    } 
    else if (key == '=') {  
        double result = evaluateExpression(input);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(input);
        lcd.setCursor(0, 1);
        lcd.print("= " + String(result));
        input = String(result);  
    } 
    else {  
        // ✅ Replace 's' with "sin(" and 'c' with "cos(" etc.
        if (key == 's') input += "sin(";
        else if (key == 'c') input += "cos(";
        else if (key == 't') input += "tan(";
        else if (key == 'l') input += "log(";
        else if (key == 'L') input += "ln(";
        else if (key == '!') input += "!"; // Factorial
        else input += key;  // Default case (normal characters)
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(input);
}


void setup() {
    lcd.begin(16, 2);
    lcd.print("Calculator Ready");

    for (int i = 0; i < ROWS; i++) {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], LOW);
    }
    for (int i = 0; i < COLS; i++) {
        pinMode(colPins[i], INPUT_PULLUP);
    }

    pinMode(SHIFT_BUTTON, INPUT_PULLUP);
}

void loop() {
    bool shiftPressed = (digitalRead(SHIFT_BUTTON) == LOW);  

    // Display shift status
    lcd.setCursor(0, 1);
    lcd.print(shiftPressed ? "Shift ON " : "         ");  

    for (int i = 0; i < ROWS; i++) {
        digitalWrite(rowPins[i], LOW);  // Activate row

        for (int j = 0; j < COLS; j++) {
            if (!digitalRead(colPins[j])) {  
                delay(200);  // Debounce delay
                
                // Select keymap based on shift mode
                char key = shiftPressed ? shiftKeys[i][j] : normalKeys[i][j];

                handleKeyPress(key);

                while (!digitalRead(colPins[j]));  // Wait for release
                delay(200);
            }
        }

        digitalWrite(rowPins[i], HIGH);  // Deactivate row
    }
}
