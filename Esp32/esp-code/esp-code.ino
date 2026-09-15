#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>


#define IR_PIN 4
#define SDA_PIN 8
#define SCL_PIN 9
#define MPU6050_ADDR 0x68


LiquidCrystal_I2C lcd(0x27, 16, 2);


WiFiMulti wifiMulti;


const char* SERVER_URL =
  "https://eye-blink-head-tilt-communicator.onrender.com/message";




// Wi-Fi settings


const unsigned long WIFI_CONNECT_TIMEOUT = 8000;
const unsigned long WIFI_RETRY_INTERVAL = 10000;


unsigned long lastWiFiAttempt = 0;
bool wifiWasConnected = false;




// HTTP settings


const unsigned long HTTP_TIMEOUT = 3000;




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
const unsigned long SELECTED_DISPLAY_TIME = 30000;


unsigned long lastMessageChange = 0;




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




// MPU6050


const unsigned long MPU_INTERVAL = 50;


unsigned long lastMPURead = 0;


float currentPitch = 0;
float currentRoll = 0;


bool mpuReadingValid = false;




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


bool connectWiFi() {


  Serial.println();
  Serial.println("Searching for saved Wi-Fi networks...");


  unsigned long startTime = millis();


  while (
    wifiMulti.run() != WL_CONNECTED &&
    millis() - startTime < WIFI_CONNECT_TIMEOUT
  ) {


    delay(200);
    Serial.print(".");
  }


  Serial.println();


  if (WiFi.status() == WL_CONNECTED) {


    Serial.println("Wi-Fi connected.");


    Serial.print("Connected network: ");
    Serial.println(WiFi.SSID());


    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());


    Serial.print("Signal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");


    wifiWasConnected = true;


    lastWiFiAttempt = millis();


    return true;


  } else {


    Serial.println("No saved Wi-Fi network available.");


    Serial.println(
      "System will continue normally."
    );


    wifiWasConnected = false;


    lastWiFiAttempt = millis();


    return false;
  }
}




// Maintain Wi-Fi connection


void maintainWiFi() {


  if (WiFi.status() == WL_CONNECTED) {


    if (!wifiWasConnected) {


      Serial.println();
      Serial.println("Wi-Fi connected again.");


      Serial.print("Connected network: ");
      Serial.println(WiFi.SSID());


      Serial.print("ESP32 IP: ");
      Serial.println(WiFi.localIP());


      wifiWasConnected = true;
    }


    return;
  }




  if (wifiWasConnected) {


    Serial.println();
    Serial.println("Wi-Fi connection lost.");


    wifiWasConnected = false;
  }




  if (
    millis() - lastWiFiAttempt >=
    WIFI_RETRY_INTERVAL
  ) {


    lastWiFiAttempt = millis();


    Serial.println();
    Serial.println("Searching for Wi-Fi again...");


    wifiMulti.run();
  }
}




// Send selected message to Render


void sendSocketNotification(const char* message) {


  Serial.println();
  Serial.println("Preparing cloud notification...");




  if (WiFi.status() != WL_CONNECTED) {


    Serial.println("Wi-Fi not connected.");


    Serial.println(
      "Cloud notification skipped."
    );


    return;
  }




  HTTPClient http;


  Serial.println("Connecting to Render...");




  http.setConnectTimeout(HTTP_TIMEOUT);
  http.setTimeout(HTTP_TIMEOUT);


  if (!http.begin(SERVER_URL)) {


    Serial.println(
      "Could not start HTTP connection."
    );


    Serial.println(
      "Local communicator continues normally."
    );


    return;
  }




  http.addHeader(
    "Content-Type",
    "application/json"
  );




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
      Serial.println(
        "MESSAGE SENT TO RENDER SUCCESSFULLY"
      );


    } else {


      Serial.println();
      Serial.println(
        "SERVER RETURNED AN ERROR"
      );
    }


  } else {


    Serial.print(
      "HTTP request failed: "
    );


    Serial.println(
      http.errorToString(httpCode)
    );


    Serial.println(
      "Local communicator continues normally."
    );
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


  Serial.print(
    "SELECTED MESSAGE: "
  );


  Serial.println(
    messages[currentMessage]
  );


  Serial.println();




  displaySelectedMessage();




  eyesClosed = false;


  resetBlinkCount();


  resetHeadMovement();


  actionMode = READY;




  Serial.println(
    "Selected message will remain"
  );


  Serial.println(
    "on LCD for 30 seconds."
  );




  // Send selected message to cloud server


  sendSocketNotification(
    messages[currentMessage]
  );
}




// Read MPU6050 acceleration


bool readMPU(
  int16_t &ax,
  int16_t &ay,
  int16_t &az
) {


  Wire.beginTransmission(
    MPU6050_ADDR
  );


  Wire.write(0x3B);




  if (
    Wire.endTransmission(false) != 0
  ) {


    return false;
  }




  uint8_t received =
    Wire.requestFrom(
      MPU6050_ADDR,
      6
    );




  if (
    received != 6 ||
    Wire.available() < 6
  ) {


    return false;
  }




  ax =
    (int16_t)(
      (Wire.read() << 8) |
      Wire.read()
    );




  ay =
    (int16_t)(
      (Wire.read() << 8) |
      Wire.read()
    );




  az =
    (int16_t)(
      (Wire.read() << 8) |
      Wire.read()
    );




  return true;
}




// Calculate pitch and roll


bool getAngles(
  float &pitch,
  float &roll
) {


  int16_t ax;
  int16_t ay;
  int16_t az;




  if (
    !readMPU(
      ax,
      ay,
      az
    )
  ) {


    return false;
  }




  pitch =
    atan2(
      (float)ax,
      sqrt(
        (float)ay * ay +
        (float)az * az
      )
    )
    * 180.0 / PI;




  roll =
    atan2(
      (float)ay,
      sqrt(
        (float)ax * ax +
        (float)az * az
      )
    )
    * 180.0 / PI;




  return true;
}




// Calibrate neutral head position


void calibrateNeutral() {


  Serial.println();


  Serial.println(
    "Keep your head straight..."
  );


  Serial.println(
    "Calibrating..."
  );




  lcd.clear();


  lcd.setCursor(0, 0);
  lcd.print("Keep head");


  lcd.setCursor(0, 1);
  lcd.print("straight...");




  float pitchSum = 0;
  float rollSum = 0;


  int validReadings = 0;




  for (int i = 0; i < 100; i++) {


    float pitch;
    float roll;




    if (
      getAngles(
        pitch,
        roll
      )
    ) {


      pitchSum += pitch;


      rollSum += roll;


      validReadings++;
    }




    delay(20);
  }




  if (validReadings > 0) {


    basePitch =
      pitchSum /
      validReadings;


    baseRoll =
      rollSum /
      validReadings;


  } else {


    basePitch = 0;
    baseRoll = 0;
  }




  Serial.println(
    "Calibration complete."
  );




  Serial.print(
    "Base Pitch: "
  );


  Serial.println(
    basePitch
  );




  Serial.print(
    "Base Roll: "
  );


  Serial.println(
    baseRoll
  );




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


  delay(1000);




  pinMode(
    IR_PIN,
    INPUT
  );




  Wire.begin(
    SDA_PIN,
    SCL_PIN
  );




  lcd.init();


  lcd.backlight();


  lcd.clear();




  lcd.setCursor(0, 0);
  lcd.print("Head / Eye");


  lcd.setCursor(0, 1);
  lcd.print("Communicator");




  delay(1500);




  // Add four Wi-Fi networks
  // Replace these placeholders with your own credentials


  wifiMulti.addAP(
    "Sarwar",
    "12345678"
  );


  wifiMulti.addAP(
    "WiFi_2",
    "PASSWORD_2"
  );


  wifiMulti.addAP(
    "WiFi_3",
    "PASSWORD_3"
  );


  wifiMulti.addAP(
    "WiFi_4",
    "PASSWORD_4"
  );




  // Wake up MPU6050


  Wire.beginTransmission(
    MPU6050_ADDR
  );


  Wire.write(0x6B);


  Wire.write(0);


  Wire.endTransmission();




  Serial.println();


  Serial.println(
    "Eye Blink / Head Tilt"
  );


  Serial.println(
    "Communicator Started"
  );




  delay(1000);




  // Try to connect to Wi-Fi


  connectWiFi();




  // Calibrate MPU6050


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




  actionMode =
    READY;




  lastMPURead =
    millis();
}




// Main loop


void loop() {


  unsigned long currentTime =
    millis();




  // Maintain Wi-Fi


  maintainWiFi();




  // Handle selected message timeout


  if (messageSelected) {


    if (
      currentTime -
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


      actionMode =
        READY;




      lastMessageChange =
        currentTime;




      Serial.print(
        "Next message: "
      );


      Serial.println(
        messages[currentMessage]
      );




      displayMessage();
    }




    return;
  }




  // Read TCRT5000


  int sensor =
    digitalRead(IR_PIN);




  // Eye blink detection


  if (
    actionMode != HEAD_ACTION
  ) {


    if (sensor == HIGH) {


      if (!eyesClosed) {


        eyesClosed = true;


        eyeCloseStart =
          currentTime;




        Serial.println(
          "EYES CLOSED"
        );




        actionMode =
          EYE_ACTION;
      }


    } else {


      if (eyesClosed) {


        unsigned long closeDuration =
          currentTime -
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




        // Normal blink


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




        // Long blink / long eye closure


        else if (
          closeDuration >=
          MIN_SELECT_TIME &&
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




        // Short intentional blink


        else if (
          closeDuration <
          MIN_SELECT_TIME
        ) {


          currentTime =
            millis();




          // First blink


          if (
            blinkCount == 0
          ) {


            blinkCount = 1;


            firstBlinkTime =
              currentTime;


            lastMessageChange =
              currentTime;


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




          // Additional blinks


          else {


            blinkCount++;




            Serial.print(
              "INTENTIONAL BLINK "
            );


            Serial.println(
              blinkCount
            );




            // Second blink


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
                "Waiting for Blink 3..."
              );




              displayBlinkCount();
            }




            // Third blink


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




        // Closure longer than maximum


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
            currentTime;
        }
      }
    }




    // Blink sequence timeout


    if (
      blinkCount > 0 &&
      currentTime -
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
        currentTime;
    }
  }




  // Read MPU6050 periodically


  if (
    currentTime -
    lastMPURead >=
    MPU_INTERVAL
  ) {


    lastMPURead =
      currentTime;




    float pitch;
    float roll;




    if (
      getAngles(
        pitch,
        roll
      )
    ) {


      currentPitch =
        pitch;


      currentRoll =
        roll;


      mpuReadingValid =
        true;


    } else {


      mpuReadingValid =
        false;
    }
  }




  // Head tilt detection


  if (
    actionMode != EYE_ACTION &&
    mpuReadingValid
  ) {


    float pitchChange =
      fabs(
        currentPitch -
        basePitch
      );




    float rollChange =
      fabs(
        currentRoll -
        baseRoll
      );




    float movementAmount =
      max(
        pitchChange,
        rollChange
      );




    // Head movement detected


    if (
      movementAmount >=
      TILT_THRESHOLD
    ) {


      if (!headTilted) {


        headTilted = true;




        Serial.println();


        Serial.println(
          "HEAD MOVEMENT DETECTED"
        );




        // First movement


        if (
          !headCountingActive
        ) {


          actionMode =
            HEAD_ACTION;


          headCountingActive =
            true;


          headMoveCount = 1;


          firstHeadMoveTime =
            currentTime;


          lastMessageChange =
            currentTime;




          Serial.println(
            "HEAD MOVEMENT 1"
          );




          Serial.println(
            "Message paused."
          );




          Serial.println(
            "Waiting 5 seconds for Movement 2..."
          );




          displayHeadCount();
        }




        // Additional movements


        else {


          headMoveCount++;




          Serial.print(
            "HEAD MOVEMENT "
          );


          Serial.println(
            headMoveCount
          );




          // Second movement


          if (
            headMoveCount == 2
          ) {


            Serial.println(
              "Movement 2 detected."
            );




            Serial.println(
              "Same message maintained."
            );




            Serial.println(
              "Waiting for Movement 3..."
            );




            displayHeadCount();
          }




          // Third movement


          else if (
            headMoveCount >=
            REQUIRED_MOVEMENTS
          ) {


            Serial.println(
              "Movement 3 detected."
            );




            Serial.println(
              "Same message maintained."
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




    // Head returned to neutral


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




    // Head movement sequence timeout


    if (headCountingActive) {


      if (
        currentTime -
        firstHeadMoveTime >
        MAX_HEAD_WINDOW
      ) {


        Serial.println();


        Serial.println(
          "HEAD MOVEMENT WAITING TIME EXPIRED"
        );




        Serial.println(
          "Movement sequence reset."
        );




        resetHeadMovement();


        actionMode =
          READY;




        displayMessage();




        lastMessageChange =
          currentTime;




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
    currentTime -
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




    displayMessage();




    lastMessageChange =
      currentTime;
  }




  delay(10);
}



