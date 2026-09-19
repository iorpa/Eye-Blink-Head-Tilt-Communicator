#include <WiFi.h>
#include <Wire.h>
#include <math.h>
#include "esp_wifi.h"
#include <WiFiClientSecure.h>


#define IR_PIN 4
#define SDA_PIN 8
#define SCL_PIN 9
#define MPU6050_ADDR 0x68


const char* WIFI_SSID = "Sarwar";
const char* WIFI_PASSWORD = "12345678";


const char* ESP01_IP = "192.168.1.16";
const uint16_t ESP01_PORT = 80;


const char* SERVER_HOST = "eye-blink-head-tilt-communicator.onrender.com";
const uint16_t SERVER_PORT = 443;


WiFiClient client;


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


bool eyesClosed = false;
unsigned long eyeCloseStart = 0;

int blinkCount = 0;
unsigned long firstBlinkTime = 0;


const unsigned long NORMAL_BLINK_TIME = 300;


const unsigned long MIN_SELECT_TIME = 2000;
const unsigned long MAX_SELECT_TIME = 4000;
const int REQUIRED_BLINKS = 3;
const unsigned long MAX_BLINK_WINDOW = 5000;


// HEAD TILT VARIABLES
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

bool messageSelected = false;
unsigned long selectionStartTime = 0;


enum ActionMode {
  READY,
  EYE_ACTION,
  HEAD_ACTION
};


ActionMode actionMode = READY;


void connectToWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }


  Serial.println();
  Serial.println("Connecting to Wi-Fi...");
  Serial.print("Connecting to: ");
  Serial.println(WIFI_SSID);


  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);


  esp_err_t txResult =
    esp_wifi_set_max_tx_power(WIFI_POWER_8_5dBm);


  if (txResult == ESP_OK) {
    Serial.println("Wi-Fi TX power set to 8.5 dBm.");
  }


  delay(500);


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


  unsigned long startAttempt = millis();


  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - startAttempt < 15000
  ) {

    delay(500);

    Serial.print(".");
    Serial.print(" Status=");
    Serial.println(WiFi.status());
  }


  Serial.println();


  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("Wi-Fi CONNECTED!");


    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());


    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());


    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());


    Serial.print("Signal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

  } else {

    Serial.println("Wi-Fi connection FAILED.");

    Serial.print("Final status: ");
    Serial.println(WiFi.status());
  }
}


// SEND DATA TO ESP-01S
void sendWirelessData(String data) {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Wi-Fi disconnected.");
    Serial.println("Trying to reconnect...");


    connectToWiFi();


    if (WiFi.status() != WL_CONNECTED) {

      Serial.println("Wi-Fi reconnect FAILED.");

      return;
    }
  }


  Serial.print("Sending wireless data: ");
  Serial.println(data);


  if (client.connect(ESP01_IP, ESP01_PORT)) {

    Serial.println("ESP-01S connected!");


    client.print(data);


    Serial.print("Sent: ");
    Serial.println(data);


    delay(500);


    client.stop();


    Serial.println("Connection closed.");

  } else {

    Serial.println("ESP-01S connection FAILED!");
  }
}


// SEND MESSAGE TO WEBSITE
void sendWebsiteMessage() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Wi-Fi disconnected.");
    Serial.println("Trying to reconnect...");


    connectToWiFi();


    if (WiFi.status() != WL_CONNECTED) {

      Serial.println("Website notification FAILED.");

      return;
    }
  }


  WiFiClientSecure secureClient;

  secureClient.setInsecure();


  Serial.println("Connecting to website server...");


  if (!secureClient.connect(
    SERVER_HOST,
    SERVER_PORT
  )) {

    Serial.println(
      "Website server connection FAILED!"
    );

    return;
  }


  String message = messages[currentMessage];


  String requestBody =
    "{\"message\":\"" +
    message +
    "\"}";


  secureClient.println(
    "POST /message HTTP/1.1"
  );

  secureClient.print(
    "Host: "
  );

  secureClient.println(
    SERVER_HOST
  );

  secureClient.println(
    "Content-Type: application/json"
  );

  secureClient.print(
    "Content-Length: "
  );

  secureClient.println(
    requestBody.length()
  );

  secureClient.println(
    "Connection: close"
  );

  secureClient.println();

  secureClient.println(
    requestBody
  );


  Serial.print(
    "Website message sent: "
  );

  Serial.println(
    message
  );


  unsigned long startTime =
    millis();


  while (
    secureClient.connected() &&
    millis() - startTime < 3000
  ) {

    while (secureClient.available()) {

      String response =
        secureClient.readStringUntil('\n');

      Serial.println(response);
    }
  }


  secureClient.stop();


  Serial.println(
    "Website connection closed."
  );
}


// SEND CURRENT MESSAGE
void sendCurrentMessage() {

  String data = "N";

  data += String(currentMessage + 1);

  sendWirelessData(data);
}


// SEND BLINK STATUS
void sendBlinkStatus(int count) {

  String data = "B";

  data += String(count);

  sendWirelessData(data);
}


// SEND HEAD STATUS
void sendHeadStatus(int count) {

  String data = "H";

  data += String(count);

  sendWirelessData(data);
}


// SEND SELECTED MESSAGE
void sendSelectedMessage() {

  String data = "S";

  data += String(currentMessage + 1);

  sendWirelessData(data);

  sendWebsiteMessage();
}


// DISPLAY MESSAGE
void displayMessage() {

  sendCurrentMessage();
}


// DISPLAY SELECTED MESSAGE
void displaySelectedMessage() {

  sendSelectedMessage();
}


// DISPLAY BLINK COUNT

void displayBlinkCount() {

  sendBlinkStatus(blinkCount);
}


// RESET BLINK SEQUENCE
void resetBlinkCount() {

  blinkCount = 0;

  firstBlinkTime = 0;
}


// RESET HEAD MOVEMENT SEQUENCE
void resetHeadMovement() {

  headMoveCount = 0;

  headCountingActive = false;

  headTilted = false;

  firstHeadMoveTime = 0;
}


// SELECT CURRENT MESSAGE
void selectMessage() {

  if (messageSelected) {
    return;
  }


  messageSelected = true;

  selectionStartTime = millis();


  Serial.println();
  Serial.println("SELECTED MESSAGE:");
  Serial.println(messages[currentMessage]);


  Serial.print("MESSAGE CODE: ");
  Serial.println(currentMessage + 1);


  displaySelectedMessage();


  eyesClosed = false;

  resetBlinkCount();

  resetHeadMovement();

  actionMode = READY;


  Serial.println("Selected message active for 30 seconds.");
}


// READ MPU6050
void readMPU(
  int16_t &ax,
  int16_t &ay,
  int16_t &az
) {

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


// CALCULATE PITCH AND ROLL
void getAngles(
  float &pitch,
  float &roll
) {

  int16_t ax;
  int16_t ay;
  int16_t az;


  readMPU(ax, ay, az);


  pitch =
    atan2(
      ax,
      sqrt(
        (float)ay * ay +
        (float)az * az
      )
    )
    * 180.0 / PI;


  roll =
    atan2(
      ay,
      sqrt(
        (float)ax * ax +
        (float)az * az
      )
    )
    * 180.0 / PI;
}


// CALIBRATE NEUTRAL HEAD POSITION
void calibrateNeutral() {

  Serial.println();

  Serial.println("Keep your head straight...");

  Serial.println("Calibrating...");


  float pitchSum = 0;

  float rollSum = 0;


  for (int i = 0; i < 100; i++) {

    float pitch;

    float roll;


    getAngles(
      pitch,
      roll
    );


    pitchSum += pitch;

    rollSum += roll;


    delay(20);
  }


  basePitch =
    pitchSum / 100.0;


  baseRoll =
    rollSum / 100.0;


  Serial.println("Calibration complete.");


  Serial.print("Base Pitch: ");

  Serial.println(basePitch);


  Serial.print("Base Roll: ");

  Serial.println(baseRoll);


  delay(1500);
}


// SETUP
void setup() {

  Serial.begin(115200);

  delay(2000);


  pinMode(IR_PIN, INPUT);


  Wire.begin(
    SDA_PIN,
    SCL_PIN
  );


  Wire.beginTransmission(
    MPU6050_ADDR
  );

  Wire.write(0x6B);

  Wire.write(0);

  Wire.endTransmission(true);


  Serial.println();

  Serial.println(
    "Eye Blink / Head Tilt Communicator"
  );

  Serial.println(
    "Starting..."
  );


  delay(1000);


  connectToWiFi();


  delay(1000);


  calibrateNeutral();


  Serial.print(
    "Current message: "
  );

  Serial.println(
    messages[currentMessage]
  );


  displayMessage();


  lastMessageChange =
    millis();


  actionMode = READY;
}


// MAIN LOOP
void loop() {


  // SELECTED MESSAGE - KEEP FOR 30 SECONDS
  if (messageSelected) {

    if (
      millis() -
      selectionStartTime >=
      SELECTED_DISPLAY_TIME
    ) {

      currentMessage++;


      if (
        currentMessage >=
        messageCount
      ) {

        currentMessage = 0;
      }


      messageSelected = false;


      resetBlinkCount();

      resetHeadMovement();


      eyesClosed = false;

      actionMode = READY;


      lastMessageChange =
        millis();


      Serial.print(
        "Next message: "
      );

      Serial.println(
        messages[currentMessage]
      );


      displayMessage();
    }


    delay(20);

    return;
  }


  // READ EYE SENSOR
  int sensor =
    digitalRead(IR_PIN);


  if (actionMode != HEAD_ACTION) {


    if (sensor == HIGH) {

      if (!eyesClosed) {

        eyesClosed = true;

        eyeCloseStart =
          millis();


        Serial.println(
          "EYES CLOSED"
        );


        actionMode =
          EYE_ACTION;
      }
    }

    else {

      if (eyesClosed) {

        unsigned long closeDuration =
          millis() -
          eyeCloseStart;


        Serial.print(
          "CLOSED FOR: "
        );

        Serial.print(
          closeDuration
        );

        Serial.println(
          " ms"
        );


        if (
          closeDuration <
          NORMAL_BLINK_TIME
        ) {

          Serial.println(
            "NORMAL BLINK - IGNORED"
          );


          eyesClosed = false;

          actionMode =
            READY;
        }

        else if (
          closeDuration >=
          MIN_SELECT_TIME
          &&
          closeDuration <=
          MAX_SELECT_TIME
        ) {

          Serial.println(
            "LONG CLOSURE - SELECT"
          );


          eyesClosed = false;


          resetBlinkCount();


          selectMessage();


          return;
        }

        else if (
          closeDuration <
          MIN_SELECT_TIME
        ) {

          unsigned long currentTime =
            millis();


          if (
            blinkCount == 0
          ) {

            blinkCount = 1;

            firstBlinkTime =
              currentTime;


            lastMessageChange =
              millis();


            actionMode =
              EYE_ACTION;


            Serial.println(
              "INTENTIONAL BLINK 1"
            );


            Serial.println(
              "Message paused."
            );


            Serial.println(
              "Waiting 5 seconds for Blink 2..."
            );


            displayBlinkCount();
          }


          else {

            blinkCount++;


            Serial.print(
              "INTENTIONAL BLINK "
            );

            Serial.println(
              blinkCount
            );


            if (
              blinkCount == 2
            ) {

              Serial.println(
                "Blink 2 detected."
              );


              Serial.println(
                "Same message maintained."
              );


              Serial.println(
                "Waiting 5 seconds for Blink 3..."
              );


              displayBlinkCount();
            }


            else if (
              blinkCount >=
              REQUIRED_BLINKS
            ) {

              Serial.println(
                "Blink 3 detected."
              );


              Serial.println(
                "Same message maintained."
              );


              Serial.println(
                "3 INTENTIONAL BLINKS - SELECT"
              );


              eyesClosed = false;


              resetBlinkCount();


              selectMessage();


              return;
            }
          }


          eyesClosed = false;


          if (
            blinkCount <
            REQUIRED_BLINKS
          ) {

            actionMode =
              EYE_ACTION;
          }
        }


        else {

          Serial.println(
            "CLOSURE > 4 SECONDS - IGNORED"
          );


          eyesClosed = false;


          resetBlinkCount();


          actionMode =
            READY;


          displayMessage();


          lastMessageChange =
            millis();
        }
      }
    }


    if (
      blinkCount > 0
      &&
      millis() -
      firstBlinkTime >
      MAX_BLINK_WINDOW
    ) {

      Serial.println();

      Serial.println(
        "BLINK WAITING TIME EXPIRED"
      );


      Serial.println(
        "Blink sequence reset."
      );


      resetBlinkCount();


      actionMode =
        READY;


      displayMessage();


      lastMessageChange =
        millis();
    }
  }


  if (
    actionMode !=
    EYE_ACTION
  ) {

    float pitch;

    float roll;


    getAngles(
      pitch,
      roll
    );


    float pitchChange =
      abs(
        pitch -
        basePitch
      );


    float rollChange =
      abs(
        roll -
        baseRoll
      );


    float movementAmount =
      max(
        pitchChange,
        rollChange
      );


    if (
      movementAmount >=
      TILT_THRESHOLD
    ) {

      if (!headTilted) {

        headTilted = true;


        Serial.println(
          "HEAD MOVEMENT DETECTED"
        );


        if (
          !headCountingActive
        ) {

          actionMode =
            HEAD_ACTION;


          headCountingActive =
            true;


          headMoveCount = 1;


          firstHeadMoveTime =
            millis();


          lastMessageChange =
            millis();


          Serial.println(
            "HEAD MOVEMENT 1"
          );


          Serial.println(
            "Message paused."
          );


          sendHeadStatus(1);
        }


        else {

          headMoveCount++;


          firstHeadMoveTime =
            millis();


          Serial.print(
            "HEAD MOVEMENT "
          );

          Serial.println(
            headMoveCount
          );


          if (
            headMoveCount == 2
          ) {

            Serial.println(
              "Movement 2 detected."
            );


            sendHeadStatus(2);
          }


          else if (
            headMoveCount >=
            REQUIRED_MOVEMENTS
          ) {

            Serial.println(
              "Movement 3 detected."
            );


            Serial.println(
              "3 HEAD MOVEMENTS - SELECT"
            );


            selectMessage();


            return;
          }
        }
      }
    }


    if (
      movementAmount <=
      NEUTRAL_THRESHOLD
    ) {

      if (headTilted) {

        headTilted = false;


        Serial.println(
          "HEAD RETURNED TO NEUTRAL"
        );
      }
    }


    if (
      headCountingActive
    ) {

      if (
        millis() -
        firstHeadMoveTime >
        MAX_HEAD_WINDOW
      ) {

        Serial.println(
          "HEAD MOVEMENT SEQUENCE TIMEOUT"
        );


        resetHeadMovement();


        actionMode =
          READY;


        displayMessage();


        lastMessageChange =
          millis();


        return;
      }
    }
  }


  if (
    actionMode == READY
    &&
    !eyesClosed
    &&
    blinkCount == 0
    &&
    !headCountingActive
    &&
    !headTilted
    &&
    millis() -
    lastMessageChange >=
    MESSAGE_INTERVAL
  ) {

    currentMessage++;


    if (
      currentMessage >=
      messageCount
    ) {

      currentMessage = 0;
    }


    Serial.print(
      "Current message: "
    );


    Serial.println(
      messages[currentMessage]
    );


    lastMessageChange =
      millis();


    displayMessage();
  }


  delay(20);
}