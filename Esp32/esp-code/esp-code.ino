
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#define IR_PIN 4
#define SDA_PIN 8
#define SCL_PIN 9
#define MPU6050_ADDR 0x68

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Wi-Fi settings
const char* WIFI_NAME = "Sarwar";
const char* WIFI_PASSWORD = "12345678";

const char* SERVER_URL =
  "https://eye-blink-head-tilt-communicator.onrender.com/message";

const unsigned long WIFI_RETRY_INTERVAL = 5000;
const unsigned long HTTP_TIMEOUT = 3000;

unsigned long lastWiFiAttempt = 0;
bool wifiConnected = false;

// Fixed messages
const char* messages[] = {
  "Need Water",
  "Call Nurse",
  "In Pain",
  "Reposition",
  "Hot / Cold",
  "Yes",
  "No",
  "Emergency"
};

const int messageCount = 8;
int currentMessage = 0;

const unsigned long MESSAGE_INTERVAL = 3000;
unsigned long lastMessageChange = 0;

const unsigned long SELECTED_DISPLAY_TIME = 30000;

// Eye blink variables
bool eyesClosed = false;
unsigned long eyeCloseStart = 0;

int blinkCount = 0;
unsigned long firstBlinkTime = 0;

const unsigned long NORMAL_BLINK_TIME = 300;
const unsigned long MIN_SELECT_TIME = 2000;
const unsigned long MAX_SELECT_TIME = 4000;

const int REQUIRED_BLINKS = 3;
const unsigned long MAX_BLINK_WINDOW = 5000;

// Head tilt variables
const float TILT_THRESHOLD = 25.0;
const float NEUTRAL_THRESHOLD = 10.0;

const int REQUIRED_MOVEMENTS = 3;
const unsigned long MAX_HEAD_WINDOW = 5000;

int headMoveCount = 0;
bool headCountingActive = false;
bool headTilted = false;
unsigned long firstHeadMoveTime = 0;

float basePitch = 0;
float baseRoll = 0;

// Message selection
bool messageSelected = false;
unsigned long selectionStartTime = 0;

// Action mode
enum ActionMode {
  READY,
  EYE_ACTION,
  HEAD_ACTION
};

ActionMode actionMode = READY;

// Display current message
void displayMessage() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Current Message");

  lcd.setCursor(0, 1);
  lcd.print(messages[currentMessage]);
}

// Display selected message
void displaySelectedMessage() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SELECTED:");

  lcd.setCursor(0, 1);
  lcd.print(messages[currentMessage]);
}

// Display blink count
void displayBlinkCount() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Blink ");
  lcd.print(blinkCount);
  lcd.print("/3");

  lcd.setCursor(0, 1);
  lcd.print(messages[currentMessage]);
}

// Display head movement count
void displayHeadCount() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Movement ");
  lcd.print(headMoveCount);
  lcd.print("/3");

  lcd.setCursor(0, 1);
  lcd.print(messages[currentMessage]);
}

// Reset blink sequence
void resetBlinkCount() {
  blinkCount = 0;
  firstBlinkTime = 0;
}

// Reset head movement sequence
void resetHeadMovement() {
  headMoveCount = 0;
  headCountingActive = false;
  headTilted = false;
  firstHeadMoveTime = 0;
}

// Connect to Wi-Fi
void connectWiFi() {
  Serial.println();
  Serial.println("Connecting to Wi-Fi...");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    Serial.print(".");

    lcd.setCursor(0, 1);
    lcd.print("Trying...       ");
  }

  Serial.println();
  Serial.println("Wi-Fi connected.");

  Serial.print("Connected network: ");
  Serial.println(WiFi.SSID());

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("Signal: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  wifiConnected = true;
  lastWiFiAttempt = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(1000);
}

// Maintain Wi-Fi connection
void maintainWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiConnected) {
      Serial.println();
      Serial.println("Wi-Fi connected again.");

      Serial.print("Connected network: ");
      Serial.println(WiFi.SSID());

      Serial.print("ESP32 IP: ");
      Serial.println(WiFi.localIP());

      wifiConnected = true;
    }

    return;
  }

  if (wifiConnected) {
    Serial.println();
    Serial.println("Wi-Fi connection lost.");
    Serial.println("Trying to reconnect...");
    wifiConnected = false;
  }

  if (millis() - lastWiFiAttempt >= WIFI_RETRY_INTERVAL) {
    lastWiFiAttempt = millis();

    Serial.println();
    Serial.println("Trying to reconnect to Wi-Fi...");

    WiFi.disconnect();
    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  }
}

// Send selected message to Render
void sendSocketNotification(const char* message) {
  Serial.println();
  Serial.println("Preparing cloud notification...");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi not connected.");
    Serial.println("Cloud notification skipped.");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  Serial.println("Connecting to Render...");

  http.setConnectTimeout(HTTP_TIMEOUT);
  http.setTimeout(HTTP_TIMEOUT);

  if (!http.begin(client, SERVER_URL)) {
    Serial.println("Could not start HTTPS connection.");
    Serial.println("Local communicator continues normally.");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  String jsonData = "{\"message\":\"";
  jsonData += message;
  jsonData += "\"}";

  Serial.println("Sending:");
  Serial.println(jsonData);

  int httpCode = http.POST(jsonData);

  if (httpCode > 0) {
    Serial.print("HTTP response code: ");
    Serial.println(httpCode);

    String response = http.getString();

    Serial.println("Server response:");
    Serial.println(response);

    if (httpCode == 200) {
      Serial.println();
      Serial.println("MESSAGE SENT TO RENDER SUCCESSFULLY");
    } else {
      Serial.println();
      Serial.println("SERVER RETURNED AN ERROR");
    }
  } else {
    Serial.print("HTTPS request failed: ");
    Serial.println(http.errorToString(httpCode));
    Serial.println("Local communicator continues normally.");
  }

  http.end();
}

// Select current message
void selectMessage() {
  if (messageSelected) {
    return;
  }

  messageSelected = true;
  selectionStartTime = millis();

  Serial.println();
  Serial.println("SELECTED MESSAGE:");
  Serial.println(messages[currentMessage]);
  Serial.println();

  displaySelectedMessage();

  eyesClosed = false;
  resetBlinkCount();
  resetHeadMovement();

  actionMode = READY;

  Serial.println("Selected message will remain");
  Serial.println("on LCD for 30 seconds.");

  // Send selected message to cloud server
  sendSocketNotification(messages[currentMessage]);

  delay(SELECTED_DISPLAY_TIME);

  currentMessage++;

  if (currentMessage >= messageCount) {
    currentMessage = 0;
  }

  messageSelected = false;

  resetBlinkCount();
  resetHeadMovement();

  eyesClosed = false;
  actionMode = READY;

  lastMessageChange = millis();

  Serial.print("Next message: ");
  Serial.println(messages[currentMessage]);

  displayMessage();
}

// Read MPU6050 acceleration
void readMPU(int16_t &ax, int16_t &ay, int16_t &az) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU6050_ADDR, 6);

  if (Wire.available() == 6) {
    ax = Wire.read() << 8 | Wire.read();
    ay = Wire.read() << 8 | Wire.read();
    az = Wire.read() << 8 | Wire.read();
  }
}

// Calculate pitch and roll
void getAngles(float &pitch, float &roll) {
  int16_t ax;
  int16_t ay;
  int16_t az;

  readMPU(ax, ay, az);

  pitch = atan2(
    ax,
    sqrt((float)ay * ay + (float)az * az)
  ) * 180.0 / PI;

  roll = atan2(
    ay,
    sqrt((float)ax * ax + (float)az * az)
  ) * 180.0 / PI;
}

// Calibrate neutral head position
void calibrateNeutral() {
  Serial.println();
  Serial.println("Keep your head straight...");
  Serial.println("Calibrating...");

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Keep head");

  lcd.setCursor(0, 1);
  lcd.print("straight...");

  float pitchSum = 0;
  float rollSum = 0;

  for (int i = 0; i < 100; i++) {
    float pitch;
    float roll;

    getAngles(pitch, roll);

    pitchSum += pitch;
    rollSum += roll;

    delay(20);
  }

  basePitch = pitchSum / 100.0;
  baseRoll = rollSum / 100.0;

  Serial.println("Calibration complete.");

  Serial.print("Base Pitch: ");
  Serial.println(basePitch);

  Serial.print("Base Roll: ");
  Serial.println(baseRoll);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Calibration");

  lcd.setCursor(0, 1);
  lcd.print("Complete");

  delay(1500);
}

// Setup
void setup() {
  Serial.begin(115200);

  pinMode(IR_PIN, INPUT);

  Wire.begin(SDA_PIN, SCL_PIN);

  lcd.init();
  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Head / Eye");

  lcd.setCursor(0, 1);
  lcd.print("Communicator");

  delay(1500);

  // Connect to the only configured Wi-Fi
  // The ESP32 keeps trying until connected
  connectWiFi();

  // Wake up MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  Serial.println();
  Serial.println("Eye Blink / Head Tilt");
  Serial.println("Communicator Started");

  delay(1000);

  // Calibrate MPU6050
  calibrateNeutral();

  Serial.print("Current message: ");
  Serial.println(messages[currentMessage]);

  displayMessage();

  lastMessageChange = millis();

  actionMode = READY;
}

// Main loop
void loop() {
  maintainWiFi();

  if (messageSelected) {
    return;
  }

  int sensor = digitalRead(IR_PIN);

  // Eye blink detection
  if (actionMode != HEAD_ACTION) {
    if (sensor == HIGH) {
      if (!eyesClosed) {
        eyesClosed = true;
        eyeCloseStart = millis();

        Serial.println("EYES CLOSED");

        actionMode = EYE_ACTION;
      }
    } else {
      if (eyesClosed) {
        unsigned long closeDuration =
          millis() - eyeCloseStart;

        Serial.print("CLOSED FOR: ");
        Serial.print(closeDuration);
        Serial.println(" ms");

        // Normal blink
        if (closeDuration < NORMAL_BLINK_TIME) {
          Serial.println("NORMAL BLINK - IGNORED");

          eyesClosed = false;
          actionMode = READY;
        }

        // Long closure
        else if (
          closeDuration >= MIN_SELECT_TIME &&
          closeDuration <= MAX_SELECT_TIME
        ) {
          Serial.println("LONG CLOSURE - SELECT");

          eyesClosed = false;
          resetBlinkCount();

          selectMessage();

          return;
        }

        // Intentional short blink
        else if (closeDuration < MIN_SELECT_TIME) {
          unsigned long currentTime = millis();

          // First blink
          if (blinkCount == 0) {
            blinkCount = 1;
            firstBlinkTime = currentTime;
            lastMessageChange = millis();
            actionMode = EYE_ACTION;

            Serial.println("INTENTIONAL BLINK 1");
            Serial.println("Message paused.");
            Serial.println("Waiting 5 seconds for Blink 2...");

            displayBlinkCount();
          }

          // Second / third blink
          else {
            blinkCount++;
            firstBlinkTime = currentTime;

            Serial.print("INTENTIONAL BLINK ");
            Serial.println(blinkCount);

            // Second blink
            if (blinkCount == 2) {
              Serial.println("Blink 2 detected.");
              Serial.println("Same message maintained.");
              Serial.println("Timer restarted.");
              Serial.println("Waiting 5 seconds for Blink 3...");

              displayBlinkCount();
            }

            // Third blink
            else if (
              blinkCount >= REQUIRED_BLINKS
            ) {
              Serial.println("Blink 3 detected.");
              Serial.println("Same message maintained.");
              Serial.println("3 INTENTIONAL BLINKS - SELECT");

              eyesClosed = false;
              resetBlinkCount();

              selectMessage();

              return;
            }
          }

          eyesClosed = false;

          if (blinkCount < REQUIRED_BLINKS) {
            actionMode = EYE_ACTION;
          }
        }

        // More than 4 seconds
        else {
          Serial.println("CLOSURE > 4 SECONDS - IGNORED");

          eyesClosed = false;
          resetBlinkCount();

          actionMode = READY;

          displayMessage();

          lastMessageChange = millis();
        }
      }
    }

    // Blink timeout
    if (
      blinkCount > 0 &&
      millis() - firstBlinkTime > MAX_BLINK_WINDOW
    ) {
      Serial.println();
      Serial.println("BLINK WAITING TIME EXPIRED");
      Serial.println("Blink sequence reset.");

      resetBlinkCount();

      actionMode = READY;

      displayMessage();

      lastMessageChange = millis();
    }
  }

  // Head tilt detection
  if (actionMode != EYE_ACTION) {
    float pitch;
    float roll;

    getAngles(pitch, roll);

    float pitchChange =
      abs(pitch - basePitch);

    float rollChange =
      abs(roll - baseRoll);

    float movementAmount =
      max(pitchChange, rollChange);

    // Head movement detected
    if (movementAmount >= TILT_THRESHOLD) {
      if (!headTilted) {
        headTilted = true;

        Serial.println();
        Serial.println("HEAD MOVEMENT DETECTED");

        // First movement
        if (!headCountingActive) {
          actionMode = HEAD_ACTION;

          headCountingActive = true;

          headMoveCount = 1;

          firstHeadMoveTime = millis();

          lastMessageChange = millis();

          Serial.println("HEAD MOVEMENT 1");
          Serial.println("Message paused.");
          Serial.println("Waiting 5 seconds for Movement 2...");

          lcd.clear();

          lcd.setCursor(0, 0);
          lcd.print("Movement 1/3");

          lcd.setCursor(0, 1);
          lcd.print(messages[currentMessage]);
        }

        // Second / third movement
        else {
          headMoveCount++;
          firstHeadMoveTime = millis();

          Serial.print("HEAD MOVEMENT ");
          Serial.println(headMoveCount);

          // Second movement
          if (headMoveCount == 2) {
            Serial.println("Movement 2 detected.");
            Serial.println("Same message maintained.");
            Serial.println("Timer restarted.");
            Serial.println("Waiting 5 seconds for Movement 3...");

            lcd.clear();

            lcd.setCursor(0, 0);
            lcd.print("Movement 2/3");

            lcd.setCursor(0, 1);
            lcd.print(messages[currentMessage]);
          }

          // Third movement
          else if (
            headMoveCount >= REQUIRED_MOVEMENTS
          ) {
            Serial.println("Movement 3 detected.");
            Serial.println("Same message maintained.");
            Serial.println("3 HEAD MOVEMENTS - SELECT");

            selectMessage();

            return;
          }
        }
      }
    }

    // Return to neutral
    if (movementAmount <= NEUTRAL_THRESHOLD) {
      if (headTilted) {
        headTilted = false;

        Serial.println("HEAD RETURNED TO NEUTRAL");
      }
    }

    // Head movement waiting time
    if (headCountingActive) {
      if (
        millis() - firstHeadMoveTime >
        MAX_HEAD_WINDOW
      ) {
        Serial.println();
        Serial.println("HEAD MOVEMENT WAITING TIME EXPIRED");
        Serial.println("Movement sequence reset.");

        resetHeadMovement();

        actionMode = READY;

        displayMessage();

        lastMessageChange = millis();

        return;
      }
    }
  }

  // Normal message cycling
  if (
    actionMode == READY &&
    !eyesClosed &&
    blinkCount == 0 &&
    !headCountingActive &&
    !headTilted &&
    millis() - lastMessageChange >= MESSAGE_INTERVAL
  ) {
    currentMessage++;

    if (currentMessage >= messageCount) {
      currentMessage = 0;
    }

    Serial.print("Current message: ");
    Serial.println(messages[currentMessage]);

    displayMessage();

    lastMessageChange = millis();
  }

  delay(20);
}
