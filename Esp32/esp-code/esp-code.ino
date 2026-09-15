#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* WIFI_NAME = "Sarwar";
const char* WIFI_PASSWORD = "12345678";

const char* SERVER_URL =
  "https://eye-blink-head-tilt-communicator.onrender.com/message";

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("ESP32 HTTPS TEST");
  Serial.println("================");

  Serial.println();
  Serial.println("Connecting to Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(
    WIFI_NAME,
    WIFI_PASSWORD
  );

  unsigned long startTime = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - startTime < 15000
  ) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Wi-Fi connection FAILED.");
    Serial.println("Test stopped.");

    return;
  }

  Serial.println("Wi-Fi connected.");

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("Signal: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  sendTestMessage();
}


void sendTestMessage() {

  Serial.println();
  Serial.println("Preparing test message...");

  WiFiClientSecure client;

  client.setInsecure();

  HTTPClient http;

  Serial.println("Connecting to Render...");

  http.setConnectTimeout(10000);
  http.setTimeout(10000);

  if (
    !http.begin(
      client,
      SERVER_URL
    )
  ) {

    Serial.println();
    Serial.println("HTTPS begin FAILED.");

    return;
  }

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  String jsonData =
    "{\"message\":\"Need Water\"}";

  Serial.println();
  Serial.println("Sending:");
  Serial.println(jsonData);

  int httpCode =
    http.POST(jsonData);

  Serial.println();

  Serial.print("HTTP response code: ");
  Serial.println(httpCode);

  if (httpCode > 0) {

    String response =
      http.getString();

    Serial.println();
    Serial.println("Server response:");
    Serial.println(response);

    if (httpCode == 200) {

      Serial.println();
      Serial.println("SUCCESS!");
      Serial.println("Message was sent to Render.");

    } else {

      Serial.println();
      Serial.println("Render returned an error.");
    }

  } else {

    Serial.println();
    Serial.print("HTTPS request failed: ");
    Serial.println(
      http.errorToString(httpCode)
    );
  }

  http.end();
}


void loop() {

}