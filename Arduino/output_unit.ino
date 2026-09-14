#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* messages[] = {
  "Need Water",
  "Call Nurse",
  "In Pain",
  "Reposition",
  "Hot / Cold",
  "Yes",
  "No",
  "EMERGENCY"
};

const int MESSAGE_COUNT = 8;

void showMessage(const char* message) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SELECTED:");

  lcd.setCursor(0, 1);
  lcd.print(message);
}

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  lcd.clear();
}

void loop() {
  for (int i = 0; i < MESSAGE_COUNT; i++) {

    Serial.print("Displaying: ");
    Serial.println(messages[i]);

    showMessage(messages[i]);

    delay(3000);
  }
}