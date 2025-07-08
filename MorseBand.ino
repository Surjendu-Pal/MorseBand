#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// ----- Pin Definitions -----
const int pinDot   = 8;    // Button for DOT (.)
const int pinDash  = 9;    // Button for DASH (-)
const int pinOK    = 10;   // Button for OK (end of character)

const int ledDot   = 5;    // LED for DOT
const int ledDash  = 6;    // LED for DASH
const int ledOK    = 7;    // LED for OK

const int BT_RX    = 11;   // Arduino receives from HC-05 TX
const int BT_TX    = 12;   // Arduino sends to HC-05 RX
SoftwareSerial BTSerial(BT_RX, BT_TX);

// I2C LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Morse code table
struct Morse { const char* code; char letter; };
const Morse morseTable[] = {
  {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
  {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'},
  {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'},
  {"--", 'M'}, {"-.", 'N'}, {"---", 'O'}, {".--.", 'P'},
  {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
  {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'},
  {"-.--", 'Y'}, {"--..", 'Z'},
  {".----", '1'}, {"..---", '2'}, {"...--", '3'}, {"....-", '4'},
  {".....", '5'}, {"-....", '6'}, {"--...", '7'}, {"---..", '8'},
  {"----.", '9'}, {"-----", '0'}
};
const int morseTableSize = sizeof(morseTable) / sizeof(Morse);

String morseBuffer = "";
String decodedText = "";

// Button state tracking
bool lastDotState = HIGH;
bool lastDashState = HIGH;
bool lastOKState = HIGH;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Decode Morse to letter
char decodeMorse(const String &code) {
  for (int i = 0; i < morseTableSize; i++) {
    if (code == morseTable[i].code) return morseTable[i].letter;
  }
  return '?'; 
}

void setup() {
  // Button inputs
  pinMode(pinDot, INPUT_PULLUP);
  pinMode(pinDash, INPUT_PULLUP);
  pinMode(pinOK, INPUT_PULLUP);

  // LEDs
  pinMode(ledDot, OUTPUT);
  pinMode(ledDash, OUTPUT);
  pinMode(ledOK, OUTPUT);
  digitalWrite(ledDot, LOW);
  digitalWrite(ledDash, LOW);
  digitalWrite(ledOK, LOW);

  // Serial and Bluetooth
  Serial.begin(9600);
  BTSerial.begin(9600);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("MorseBand Ready");
  delay(1500);
  lcd.clear();
  updateLCD();
}

void loop() {
  unsigned long now = millis();

  // --- DOT ---
  bool dotState = digitalRead(pinDot);
  if (dotState == LOW && lastDotState == HIGH && (now - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = now;
    digitalWrite(ledDot, HIGH);
    morseBuffer += ".";
    Serial.print(".");
    updateLCD();
    delay(100);
    digitalWrite(ledDot, LOW);
  }
  lastDotState = dotState;

  // --- DASH ---
  bool dashState = digitalRead(pinDash);
  if (dashState == LOW && lastDashState == HIGH && (now - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = now;
    digitalWrite(ledDash, HIGH);
    morseBuffer += "-";
    Serial.print("-");
    updateLCD();
    delay(100);
    digitalWrite(ledDash, LOW);
  }
  lastDashState = dashState;

  // --- OK ---
  bool okState = digitalRead(pinOK);
  if (okState == LOW && lastOKState == HIGH && (now - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = now;
    digitalWrite(ledOK, HIGH);

    char letter = decodeMorse(morseBuffer);
    decodedText += letter;
    Serial.print(" -> ");
    Serial.println(letter);
    BTSerial.print(letter);  // send to Bluetooth

    morseBuffer = "";
    updateLCD();
    delay(200);
    digitalWrite(ledOK, LOW);
  }
  lastOKState = okState;
}

// Update LCD display
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Morse: ");
  lcd.print(morseBuffer);

  lcd.setCursor(0, 1);
  lcd.print("Text: ");
  if (decodedText.length() > 10) {
    lcd.print(decodedText.substring(decodedText.length() - 10)); // show last 10 chars
  } else {
    lcd.print(decodedText);
  }
}