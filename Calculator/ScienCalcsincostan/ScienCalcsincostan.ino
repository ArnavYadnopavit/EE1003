#include <LiquidCrystal.h>
#include <math.h>

// ---------------- LCD Setup ----------------
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
// ---------------- CORDIC Setup ----------------
// We use two fixed-point scales:
// - For outputs (sin/cos): Q15 scale, SCALE_OUT = 32768.0
// - For angles: we map 90° to ANGLE_SCALE = 16384.0
#define ITERATIONS 16
#define SCALE_OUT 32768.0      
#define ANGLE_SCALE 16384.0    
#define CORDIC_K (0.607252935 * SCALE_OUT)

const int16_t atan_table[ITERATIONS] = {
  8192, 4828, 2552, 1296, 650, 325, 163, 81,
  40, 20, 10, 5, 2, 1, 0, 0
};

// CORDIC algorithm: expects theta in fixed-point angle units (0 to ANGLE_SCALE for 0° to 90°)
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

// ---------------- Trigonometric Evaluation ----------------
// This function takes an angle (in degrees) and a function name ("sin", "cos", "tan").
// It normalizes the angle to [–90, +90] and maps it into [0,90] for CORDIC. For angles
// very close to ±90°, it bypasses CORDIC to return exact values.
double evaluateTrigFunction(String func, double angle_deg) {
  // Normalize to [-180, 180]
  while (angle_deg > 180) angle_deg -= 360;
  while (angle_deg < -180) angle_deg += 360;
  
  bool negative = false;
  if (angle_deg < 0) {
    negative = true;
    angle_deg = -angle_deg;
  }
  
  // Map angles >90° to [0,90] using sin(θ)=sin(180-θ)
  if (angle_deg > 90) {
    angle_deg = 180 - angle_deg;
  }
  
  // Bypass CORDIC if angle is extremely close to 90°:
  if (fabs(angle_deg - 90.0) < 0.001) {
    double result = (func == "sin") ? 1.0 : (func == "cos") ? 0.0 : NAN;
    if ((func == "sin" || func == "tan") && negative) result = -result;
    return result;
  }
  
  // Convert angle (0 to 90) to fixed-point: 90° -> ANGLE_SCALE
  int16_t theta = (int16_t)((angle_deg / 90.0) * ANGLE_SCALE);
  
  int16_t sin_val, cos_val;
  cordic(theta, &sin_val, &cos_val);
  
  double result = 0;
  if (func == "sin") result = (double) sin_val / SCALE_OUT;
  else if (func == "cos") result = (double) cos_val / SCALE_OUT;
  else if (func == "tan") result = (cos_val != 0) ? ((double) sin_val / cos_val) : NAN;
  
  if ((func == "sin" || func == "tan") && negative) result = -result;
  return result;
}


// ---------------- Button Matrix Setup ----------------
#define ROWS 4
#define COLS 5
#define SHIFT_BUTTON 13  // Shift button

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

// ---------------- Math Functions ----------------
double factorial(int n) {
  if (n < 0) return NAN;
  double fact = 1;
  for (int i = 1; i <= n; i++) fact *= i;
  return fact;
}

// ---------------- Expression Evaluation ----------------
double evaluateExpression(String expr) {
  expr.replace("Pi", "3.141592653589793");
  expr.replace("e", "2.718281828459045");
   if (expr.startsWith("sin(") || expr.startsWith("cos(") || expr.startsWith("tan(")) {
    int startIdx = expr.indexOf("(") + 1;
    int endIdx = expr.indexOf(")");
    if (endIdx > startIdx) {
      double angle = expr.substring(startIdx, endIdx).toDouble();
      if (expr.startsWith("sin")) return evaluateTrigFunction("sin", angle);
      if (expr.startsWith("cos")) return evaluateTrigFunction("cos", angle);
      if (expr.startsWith("tan")) return evaluateTrigFunction("tan", angle);
    }
    return NAN;
  }
  

  // Factorial (!)
  if (expr.endsWith("!")) {
    int num = expr.substring(0, expr.length() - 1).toInt();
    return factorial(num);
  }

  // Handle Absolute Value |x|
  while (expr.indexOf('|') != -1) {
    int startIdx = expr.indexOf('|');
    int endIdx = expr.indexOf('|', startIdx + 1);
    if (endIdx != -1) {
      double val = evaluateExpression(expr.substring(startIdx + 1, endIdx));
      expr = expr.substring(0, startIdx) + String(abs(val)) + expr.substring(endIdx + 1);
    } else {
      return NAN;  // Error if unmatched '|'
    }
  }

  // Handle Square (^2) and Cube (^3)
  while (expr.indexOf('^') != -1) {
    int idx = expr.indexOf('^');
    
    // Extract base number
    int baseStart = idx - 1;
    while (baseStart >= 0 && (isDigit(expr[baseStart]) || expr[baseStart] == '.')) {
      baseStart--;
    }
    baseStart++; // Adjust position

    double base = expr.substring(baseStart, idx).toDouble();

    // Extract exponent number
    int exponentEnd = idx + 1;
    while (exponentEnd < expr.length() && (isDigit(expr[exponentEnd]) || expr[exponentEnd] == '.')) {
      exponentEnd++;
    }

    double exponent = expr.substring(idx + 1, exponentEnd).toDouble();
    double result = pow(base, exponent);

    // Replace in expression
    expr = expr.substring(0, baseStart) + String(result) + expr.substring(exponentEnd);
  }

  // Handle Square Root (q) and Cube Root (b)
  if (expr.startsWith("sqrt(")) {
    int startIdx = expr.indexOf("(") + 1;
    int endIdx = expr.indexOf(")");
    if (endIdx > startIdx) {
      double val = expr.substring(startIdx, endIdx).toDouble();
      return (val >= 0) ? sqrt(val) : NAN;
    }
    return NAN;
  }

  if (expr.startsWith("cbrt(") || expr.startsWith("b(")) {
    int startIdx = expr.indexOf("(") + 1;
    int endIdx = expr.indexOf(")");
    if (endIdx > startIdx) {
      double val = expr.substring(startIdx, endIdx).toDouble();
      return pow(val, 1.0 / 3.0);
    }
    return NAN;
  }

  // Handle Rth Root (R(x,y))
  if (expr.startsWith("R(")) {
    int startIdx = expr.indexOf("(") + 1;
    int commaIdx = expr.indexOf(",");
    int endIdx = expr.indexOf(")");
    if (commaIdx > startIdx && endIdx > commaIdx) {
      double r = expr.substring(startIdx, commaIdx).toDouble();
      double x = expr.substring(commaIdx + 1, endIdx).toDouble();
      return pow(x, 1.0 / r);
    }
    return NAN;
  }

  // Inverse Trigonometry
  if (expr.startsWith("sininv(")) return asin(expr.substring(7, expr.length() - 1).toDouble()) * (180.0 / M_PI);
  if (expr.startsWith("cosinv(")) return acos(expr.substring(7, expr.length() - 1).toDouble()) * (180.0 / M_PI);
  if (expr.startsWith("taninv(")) return atan(expr.substring(7, expr.length() - 1).toDouble()) * (180.0 / M_PI);

  // Logarithms
  if (expr.startsWith("ln(")) return log(expr.substring(3, expr.length() - 1).toDouble());
  if (expr.startsWith("log(")) return log10(expr.substring(4, expr.length() - 1).toDouble());
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
  // Standard Arithmetic Evaluation
  return expr.toDouble();
}

// ---------------- Key Press Handling ----------------
void handleKeyPress(char key) {
  lcd.setCursor(0, 0);
  if (key == 'C') {
    input = "";
    lcd.clear();
  } else if (key == 'D') {
    if (input.length() > 0) input.remove(input.length() - 1);
  } else if (key == '=') {
    double result = evaluateExpression(input);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(input);
    lcd.setCursor(0, 1);
    lcd.print("= " + String(result, 6));
    input = String(result);
  } else {
    if (key == 's') input += "sin(";
    else if (key == 'c') input += "cos(";
    else if (key == 't') input += "tan(";
    else if (key == 'l') input += "log(";
    else if (key == 'L') input += "ln(";
    else if (key == '!') input += "!";
    else if (key == 'q') input += "sqrt(";
    else if (key == 'b') input += "cbrt(";
    else if (key == 'N') input += "N";
    else if (key == 'E') input += "E";
    else if (key == 'R') input += "R(";
    else if (key == 'S') input += "sininv(";
    else if (key == 'I') input += "cosinv(";
    else if (key == 'T') input += "taninv(";
    else if (key == '|') input += "|";
    else if (key == 'Q') input += "^2";  // Square
    else if (key == 'B') input += "^3";  // Cube
    else if (key == 'P') input += "Pi";
    else input += key;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(input);
}

// ---------------- Setup ----------------
bool shiftMode = false;
bool lastShiftState = HIGH;


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

// ---------------- Main Loop ----------------
void loop() {
  bool currentShiftState = digitalRead(SHIFT_BUTTON);
  if (lastShiftState == HIGH && currentShiftState == LOW) {
    shiftMode = !shiftMode;
    delay(200);
  }
  lastShiftState = currentShiftState;
  
  lcd.setCursor(0, 1);
  lcd.print(shiftMode ? "Shift ON " : "         ");
  
  for (int i = 0; i < ROWS; i++) {
    digitalWrite(rowPins[i], LOW);
    for (int j = 0; j < COLS; j++) {
      if (!digitalRead(colPins[j])) {
        delay(50);
        char key = shiftMode ? shiftKeys[i][j] : normalKeys[i][j];
        handleKeyPress(key);
        while (!digitalRead(colPins[j]));
        delay(50);
      }
    }
    digitalWrite(rowPins[i], HIGH);
  }
}
