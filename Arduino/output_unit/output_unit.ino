#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

SoftwareSerial espSerial(2, 3);

String espBuffer = "";

int currentMessageCode = 1;

const char* ESP01_WIFI_SSID = "Sarwar";
const char* ESP01_WIFI_PASSWORD = "12345678";

const char* messageNames[] = {
  "Need Water",
  "Call Nurse",
  "In Pain",
  "Reposition",
  "Hot / Cold",
  "Yes",
  "No",
  "Emergency"
};


void sendESPCommand(String command, unsigned long waitTime) {

  Serial.print("[ESP-01S CMD] ");
  Serial.println(command);

  espSerial.println(command);

  unsigned long startTime = millis();

  while (millis() - startTime < waitTime) {

    while (espSerial.available()) {

      char c = espSerial.read();

      Serial.write(c);
    }
  }

  Serial.println();
}


void setupESP01S() {

  Serial.println();
  Serial.println("Configuring ESP-01S...");

  delay(2000);

  sendESPCommand("AT", 1000);

  sendESPCommand("AT+CWMODE=1", 1000);

  String wifiCommand =
    "AT+CWJAP=\"" +
    String(ESP01_WIFI_SSID) +
    "\",\"" +
    String(ESP01_WIFI_PASSWORD) +
    "\"";

  sendESPCommand(wifiCommand, 20000);

  sendESPCommand("AT+CIFSR", 3000);

  sendESPCommand("AT+CIPMUX=1", 1000);

  sendESPCommand("AT+CIPSERVER=1,80", 1500);

  Serial.println();
  Serial.println("ESP-01S TCP SERVER READY");
  Serial.println("Port: 80");
}


void showNormalMessage(int code) {

  if (code < 1 || code > 8) {
    return;
  }

  currentMessageCode = code;

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Current Message");

  lcd.setCursor(0, 1);
  lcd.print(messageNames[code - 1]);

  Serial.print("[MESSAGE] ");
  Serial.println(messageNames[code - 1]);
}


void showBlinkStatus(int count, int code) {

  if (code < 1 || code > 8) {
    return;
  }

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Blink ");
  lcd.print(count);
  lcd.print("/3");

  lcd.setCursor(0, 1);
  lcd.print(messageNames[code - 1]);

  Serial.print("[BLINK] ");
  Serial.print(count);
  Serial.print("/3 - ");
  Serial.println(messageNames[code - 1]);
}


void showHeadStatus(int count, int code) {

  if (code < 1 || code > 8) {
    return;
  }

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Movement ");
  lcd.print(count);
  lcd.print("/3");

  lcd.setCursor(0, 1);
  lcd.print(messageNames[code - 1]);

  Serial.print("[HEAD] ");
  Serial.print(count);
  Serial.print("/3 - ");
  Serial.println(messageNames[code - 1]);
}


void showSelectedMessage(int code) {

  if (code < 1 || code > 8) {
    return;
  }

  currentMessageCode = code;

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SELECTED:");

  lcd.setCursor(0, 1);
  lcd.print(messageNames[code - 1]);

  Serial.print("[SELECTED] ");
  Serial.println(messageNames[code - 1]);
}


void processESPData(char c) {

  espBuffer += c;

  int ipdPosition = espBuffer.indexOf("+IPD,");

  if (ipdPosition != -1) {

    int colonPosition =
      espBuffer.indexOf(':', ipdPosition);

    if (colonPosition != -1) {

      int payloadStart = colonPosition + 1;

      if (espBuffer.length() >= payloadStart + 2) {

        char type =
          espBuffer.charAt(payloadStart);

        char number =
          espBuffer.charAt(payloadStart + 1);

        if (
          (type == 'N' ||
           type == 'B' ||
           type == 'H' ||
           type == 'S') &&
          number >= '1' &&
          number <= '8'
        ) {

          int code = number - '0';

          Serial.print("[ESP-01S DATA] ");
          Serial.print(type);
          Serial.println(number);

          if (type == 'N') {

            showNormalMessage(code);

          }

          else if (type == 'B') {

            showBlinkStatus(
              code,
              currentMessageCode
            );

          }

          else if (type == 'H') {

            showHeadStatus(
              code,
              currentMessageCode
            );

          }

          else if (type == 'S') {

            showSelectedMessage(code);

          }

          espBuffer = "";

          return;
        }
      }
    }
  }

  if (espBuffer.length() > 100) {
    espBuffer = "";
  }
}


void receiveESP01() {

  espSerial.listen();

  while (espSerial.available()) {

    char c = espSerial.read();

    Serial.write(c);

    processESPData(c);
  }
}


void setup() {

  Serial.begin(9600);

  espSerial.begin(9600);

  lcd.init();
  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Eye / Head");

  lcd.setCursor(0, 1);
  lcd.print("Communicator");

  delay(1500);

  Serial.println();
  Serial.println("Arduino Communicator Started");

  Serial.println();
  Serial.println("[ESP-01S] Starting serial...");
  Serial.println("[ESP-01S] RX = Arduino D2");
  Serial.println("[ESP-01S] TX = Arduino D3");
  Serial.println("[ESP-01S] Baud = 9600");

  setupESP01S();


  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Waiting for");

  lcd.setCursor(0, 1);
  lcd.print("message...");

  espSerial.listen();

  Serial.println();
  Serial.println("Waiting for ESP-01S data...");
}

void loop() {

  receiveESP01();

  delay(10);
}