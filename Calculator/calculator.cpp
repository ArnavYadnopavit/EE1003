#include <LiquidCrystal.h>
#include <math.h>  // For scientific functions

// LCD setup (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

// Button matrix setup
#define ROWS 4
#define COLS 5
#define SHIFT_BUTTON 10  // Shift button

// Row and Column Pins (unchanged from your setup)
int rowPins[ROWS] = {2, 3, 4, 5};  
int colPins[COLS] = {6, 7, 8, 9, 11};  

// Normal and Shift Key Mappings
char normalKeys[ROWS][COLS] = {
    {'1', '2', '3', '/', 'C'},  // C = Clear
    {'4', '5', '6', '*', 'S'},  // S = Shift Mode (Handled Separately)
    {'7', '8', '9', '-', 'D'},  // D = Delete (Backspace)
    {'0', '=', '.', '+', 'M'}   // M = Memory (Future feature)
};

char shiftKeys[ROWS][COLS] = {
    {'s', 'c', 't', '^', 'R'}, // sin, cos, tan, power, root
    {'(', ')', 'π', '!', 'E'}, // Parentheses, pi, factorial, Exp
    {'%', 'l', 'L', 'e', 'D'}, // Modulo, ln, log, Euler, Delete
    {'A', 'B', 'C', 'D', 'M'}  // Extra functions (customizable)
};

// Input buffer
String input = "";



// Handles key presses and updates display
void handleKeyPress(char key) {
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
        input += key;
    }

    lcd.setCursor(0, 0);
    lcd.print(input);
}

// Evaluate the mathematical expression
double evaluateExpression(String expr) {
    expr.replace("π", "3.141592653589793");
expr.replace("e", "2.718281828459045");


    double num1, num2;
    char op;
    int pos = -1;

    if ((pos = expr.indexOf('+')) > -1) {
        num1 = expr.substring(0, pos).toFloat();
        num2 = expr.substring(pos + 1).toFloat();
        return num1 + num2;
    } 
    else if ((pos = expr.indexOf('-')) > -1) {
        num1 = expr.substring(0, pos).toFloat();
        num2 = expr.substring(pos + 1).toFloat();
        return num1 - num2;
    } 
    else if ((pos = expr.indexOf('*')) > -1) {
        num1 = expr.substring(0, pos).toFloat();
        num2 = expr.substring(pos + 1).toFloat();
        return num1 * num2;
    } 
    else if ((pos = expr.indexOf('/')) > -1) {
        num1 = expr.substring(0, pos).toFloat();
        num2 = expr.substring(pos + 1).toFloat();
        if (num2 == 0) return NAN;
        return num1 / num2;
    } 
    else if (expr.startsWith("sin(") && expr.endsWith(")")) {
        return sin(expr.substring(4, expr.length() - 1).toFloat() * (PI / 180));
    } 
    else if (expr.startsWith("cos(") && expr.endsWith(")")) {
        return cos(expr.substring(4, expr.length() - 1).toFloat() * (PI / 180));
    } 
    else if (expr.startsWith("tan(") && expr.endsWith(")")) {
        return tan(expr.substring(4, expr.length() - 1).toFloat() * (PI / 180));
    } 
    else if (expr.startsWith("log(") && expr.endsWith(")")) {
        return log10(expr.substring(4, expr.length() - 1).toFloat());
    } 
    else if (expr.startsWith("ln(") && expr.endsWith(")")) {
        return log(expr.substring(3, expr.length() - 1).toFloat());
    } 
    else if (expr.startsWith("sqrt(") && expr.endsWith(")")) {
        return sqrt(expr.substring(5, expr.length() - 1).toFloat());
    } 
    else if ((pos = expr.indexOf('^')) > -1) {
        num1 = expr.substring(0, pos).toFloat();
        num2 = expr.substring(pos + 1).toFloat();
        return pow(num1, num2);
    } 

    return NAN;  // Invalid input
}
void setup() {
    lcd.begin(16, 2);
    lcd.print("Calculator Ready");

    // Setup button matrix
    for (int i = 0; i < ROWS; i++) {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], LOW);
    }
    for (int i = 0; i < COLS; i++) {
        pinMode(colPins[i], INPUT_PULLUP);
    }

    // Shift button as input pull-up
    pinMode(SHIFT_BUTTON, INPUT_PULLUP);
}

void loop() {
    bool shiftPressed = !digitalRead(SHIFT_BUTTON);

    if (shiftPressed) {
        lcd.clear();
        lcd.print("Shift ON");
        delay(500);
        lcd.clear();
    }

    for (int i = 0; i < ROWS; i++) {
        digitalWrite(rowPins[i], HIGH);

        for (int j = 0; j < COLS; j++) {
            if (!digitalRead(colPins[j])) {  
                delay(50);  // Debounce
                char key = shiftPressed ? shiftKeys[i][j] : normalKeys[i][j];

                handleKeyPress(key);

                while (!digitalRead(colPins[j]));  // Wait for button release
            }
        }
        
        digitalWrite(rowPins[i], LOW);
    }
}

