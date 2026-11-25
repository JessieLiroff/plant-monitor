unsigned long previousMillis = 0;
const long interval = 1000;

struct Sensor {
  const char* name;
  int pin;
  int dry;
  int wet;
  int needsWaterValue;
};

Sensor sensors[] = {
  { "Snake Plant", 34, 3150, 800, 10 },
  { "Ficus elastica", 35, 3190, 1050, 15 }
};



#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"

// Initialize the web server on port 80
WebServer server(80);

// Route handler for the root URL "/"
void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><title>ESP32 Server</title></head>";
  html += "<body><h1>Plant Moisture Detector</h1>";

  for (int i = 0; i < numberOfSensors(); i++) {
    int moisturePercentage = findMoisturePercentage(sensors[i], analogRead(sensors[i].pin));
    html += "<p>";
    html += sensors[i].name;
    html += " ";
    html += String(moisturePercentage);
    html += "% ";
    html += waterSensorEmoji(sensors[i].needsWaterValue, moisturePercentage);
    html += "</p>";
  };

  html += "</body></html>";

  // Arguments: HTTP Status Code, Content Type, Content Body
  server.send(200, "text/html", html);
}

void handleReadings() {
  String readings = "[";

  for (int i = 0; i < numberOfSensors(); i++) {
    int value = analogRead(sensors[i].pin);
    String name = "\"name\":\"" + String(sensors[i].name) + "\"";
    String moisturePercentage = "\"percentage\":" + String(findMoisturePercentage(sensors[i], value));
    String raw = "\"raw\":" + String(value);
    String comma = i == numberOfSensors() - 1 ? "" : ", ";

    readings += "{" + name + "," + moisturePercentage + "," + raw + "}" + comma;
  }

  readings += "]";

  server.send(200, "application/json", readings);
}

// Route handler for unmatched URLs (404 Error)
void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

int findMoisturePercentage(Sensor sensor, int value) {
  return constrain(map(value, sensor.dry, sensor.wet, 0, 100), 0, 100);
}

int numberOfSensors() {
  return sizeof(sensors) / sizeof(sensors[0]);
}

const char* waterSensorEmoji(int needsWaterValue, int percentage) {
  if (percentage >= needsWaterValue) {
    return "<span>&#128525; &#127807;</span>";
  }

  return "<span>&#128167;</span>";
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(SSID, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println(WiFi.localIP());

  // Bind route URLs to handler functions
  server.on("/", handleRoot);
  server.on("/api/readings", handleReadings);
  server.onNotFound(handleNotFound);

  // Fire up the server
  server.begin();
  Serial.println("HTTP server started.");
}


void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    for (int i = 0; i < numberOfSensors(); i++) {
      Serial.print(" ");
      Serial.print(sensors[i].name);
      Serial.print(": ");
      Serial.print(findMoisturePercentage(sensors[i], analogRead(sensors[i].pin)));
      Serial.print("%");
    }

    Serial.println("");
  }

  server.handleClient();
}
