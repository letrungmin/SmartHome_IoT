#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>


#define LED_PIN 2
#define POWER_PIN 7
#define SIGNAL_PIN A1
#define THRESHOLD 300
#define DHTPIN A2
#define DHTTYPE DHT11  // DHT 11
#include "DHT.h"

DHT dht(DHTPIN, DHTTYPE);
// Set the LCD address (you may need to change this based on your LCD module)
#define LCD_I2C_ADDR 0x27

// Create an instance for the LiquidCrystal_I2C class
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 16, 2);  // Adjust the parameters based on your LCD size

const byte rows = 4;
const byte columns = 4;
int holdDelay = 700;
int n = 3;
int state = 0;
char key = 0;
int pos = 0;
String default_passwd = "0000";
String input_passwd = "";
char lock_key = '*';
char unlock_key = '#';
char change_pass_key = '-';

Servo servo_A0;

char keys[rows][columns] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' },
};

byte rowPins[rows] = { 12, 11, 10, 9 };
byte columnPins[columns] = { 8, 7, 6, 5 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  servo_A0.attach(A0);
  dht.begin();
  servo_A0.write(pos);
  LCD_display(0, 0, "Welcome!");
}

void loop() {
  lcd.clear();
  LCD_display(0, 0, "STATUS: LOCKED!");
  LCD_display(0, 1, "UNLOCK: PRESS #");

  while (key != unlock_key && key != change_pass_key) {
    key = function_key(n);
  }

  if (key == unlock_key) {
    Unlock();
  }

  if (key == change_pass_key) {
    default_passwd = Change_password(4, default_passwd);
    delay(2000);
    key = 0;
  }
  TemHumid();
  water();
}

char function_key(int n) {
  char temp = keypad.getKey();
  if ((int)keypad.getState() == PRESSED) {
    if (temp != 0) {
      key = temp;
    }
  }
  if ((int)keypad.getState() == HOLD) {
    state++;
    state = constrain(state, 1, n);
    delay(holdDelay);
  }
  if ((int)keypad.getState() == RELEASED) {
    key += state;
    state = 0;
  }
  delay(100);
  return key;
}

String input_password(int num_char) {
  String passwd = "";
  lcd.setCursor(0, 1);
  do {
    char temp = keypad.getKey();
    if (temp != 0) {
      lcd.print("*");
      passwd += temp;
    }
    delay(100);
  } while (passwd.length() < num_char);
  return passwd;
}

String Change_password(int num_char, String current_passwd) {
  LCD_display(0, 0, "OLD PASSWORD:");
  String old_passwd = input_password(num_char);

  if (old_passwd != current_passwd) {
    lcd.clear();
    LCD_display(0, 0, "WRONG PASSWORD!");
    return current_passwd;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("NEW PASSWORD:");
  String new_passwd = input_password(num_char);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONFIRM PASSWORD:");
  String confirm_passwd = input_password(num_char);

  if (confirm_passwd == new_passwd) {
    lcd.clear();
    LCD_display(0, 0, "CHANGED PASSWORD");
    return confirm_passwd;
  } else {
    lcd.clear();
    LCD_display(0, 0, "NOTHING CHANGES!");
    return current_passwd;
  }
}

void Unlock() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INPUT PASSWORD:");
  input_passwd = input_password(4);
  if (input_passwd == default_passwd) {
    lcd.clear();
    LCD_display(0, 0, "OPENING!!!");
    delay(1000);
    for (pos = 0; pos <= 90; pos += 1) {
      servo_A0.write(pos);
      delay(15);
    }
    delay(3000);

    lcd.clear();
    LCD_display(0, 0, "CLOSING!!!");
    for (pos = 90; pos >= 0; pos -= 1) {
      servo_A0.write(pos);
      delay(15);
    }
  } else {
    LCD_display(0, 0, "WRONG PASSWORD!");
    delay(1000);
  }
  input_passwd = "";
  key = 0;
}

void LCD_display(int column, int line, String message) {
  lcd.setCursor(column, line);
  lcd.print(message);
}

int value = 0;  // variable to store the sensor value


void TemHumid() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  lcd.setCursor(0, 0);
  lcd.print("temp:");
  lcd.print(t);
  lcd.setCursor(0, 1);
  lcd.print("humid:");
  lcd.print(h);
  Serial.print("TEMP: ");
  Serial.println(t);
  Serial.print("HUMID: ");
  Serial.println(h);

  Serial.println();
  delay(1500);
}
void water() {
  digitalWrite(POWER_PIN, HIGH);   // turn the sensor ON
  delay(10);                       // wait 10 milliseconds
  value = analogRead(SIGNAL_PIN);  // read the analog value from sensor
  digitalWrite(POWER_PIN, LOW);    // turn the sensor OFF

  if (value > THRESHOLD) {
    Serial.print("The water is detected");
    digitalWrite(LED_PIN, HIGH);  // turn LED ON
    lcd.setCursor(0, 0);
    lcd.print("The water ");
    lcd.setCursor(0, 1);
    lcd.print("is detected");

  } else {
    digitalWrite(LED_PIN, LOW);  // turn LED OFF
    Serial.print("The water is not detected");
    lcd.setCursor(0, 0);
    lcd.print("The water is");
    lcd.setCursor(0, 1);
    lcd.print(" not detected");
  }
  delay(3000);
}