#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Adafruit_MotorShield.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <ArduinoJson.h> // Library for handling JSON data

const char* ssid = "GlobeAtHome_CE17E";
const char* password = "BBC930F189";

const char* host = "https://posturapp-47a59-default-rtdb.firebaseio.com/"; // Your Firebase project ID

ESP8266WebServer server(80);

// Stepper motor
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *stepper = AFMS.getStepper(200, 1);
const int stepsPerRevolution = 200;

// OLED display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LCD object
LiquidCrystal_I2C lcd(0x27, 20, 4);
// Vibrator Pin
const int vibratorPin = D0;
const int vibratorPin1 = D1;
const int vibratorPin2 = D2;
const int vibratorPin3 = D3;
const int vibratorPin4 = D4;
const int vibratorPin5 = D5;
const int vibratorPin6 = D6;
const int vibratorPin7 = D7;
const int vibratorPin8 = D8;

//Kung ilang beses po mag mag va vibrate
const int blinkCount = 5;
// Kung gaano kabilis ang vibration
const int blinkDuration = 500;


void setup() {
  Serial.begin(9600);
  delay(1000);

  // Initialize stepper motor
  AFMS.begin();
  stepper->setSpeed(100);

  // Initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("IP Address:");
  display.display();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start server and set up routes
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/notify", handleNotification);
  server.on("/data", handleDataRequest); // Route to handle data request

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = "<h1>Motor Control</h1>";
  html += "<p><a href=\"/forward\"><button>Forward</button></a></p>";
  html += "<p><a href=\"/backward\"><button>Backward</button></a></p>";
  html += "<p><a href=\"/notify\"><button>Notify</button></a></p>";
  html += "<p><a href=\"/data\"><button>Get Data</button></a></p>"; // Link to retrieve data
  server.send(200, "text/html", html);
}

void handleForward() {
  // Rotate the stepper motor in the forward direction
  stepper->setSpeed(1000); // Set the speed of the motor (in RPM)
  for (int i = 0; i < stepsPerRevolution; i++) {
    stepper->step(1, FORWARD, MICROSTEP);
  }
  server.send(200, "text/html", "<h1>Motor Forward</h1><p><a href=\"/\"><button>Back</button></a></p>");
  Serial.println("Pressed: Forward");
}

void handleBackward() {
  // Rotate the stepper motor in the backward direction
  stepper->setSpeed(1000); // Set the speed of the motor (in RPM)
  for (int i = 0; i < stepsPerRevolution; i++) {
    stepper->step(1, BACKWARD, MICROSTEP);
  }
  server.send(200, "text/html", "<h1>Motor Backward</h1><p><a href=\"/\"><button>Back</button></a></p>");
  Serial.println("Pressed: Backward");
}

void handleNotification() {
  // Turn on the vibrator for 1 second
  digitalWrite(vibratorPin, HIGH);
  delay(1000);
  digitalWrite(vibratorPin, LOW);
  server.send(200, "text/html", "<h1>Vibration On</h1><p><a href=\"/\"><button>Back</button></a></p>");
  Serial.println("Pressed: Vibration On");
}

void handleDataRequest() {
  // Making a GET request to Firebase to retrieve data
  WiFiClient client;
  if (client.connect(host, 80)) {
    client.print("GET /data/posture.json HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(host);
    client.print("\r\n");
    client.print("Connection: close\r\n\r\n");
  } else {
    Serial.println("Connection failed");
    return;
  }

  // Waiting for response
  while (!client.available() && client.connected()) {
    delay(1);
  }

  // Parsing JSON response
  const size_t capacity = JSON_OBJECT_SIZE(1) + 20;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, client);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Extracting data from JSON
  int postureValue = doc["posture"].as<int>();

  // Check if postureValue is bad
  if (postureValue == -1) { // Assuming -1 represents "bad" posture
    // Trigger vibration
    digitalWrite(vibratorPin, HIGH);
    digitalWrite(vibratorPin1, HIGH);
    digitalWrite(vibratorPin2, HIGH);
    digitalWrite(vibratorPin3, HIGH);
    digitalWrite(vibratorPin4, HIGH);
    digitalWrite(vibratorPin5, HIGH);
    digitalWrite(vibratorPin6, HIGH);
    digitalWrite(vibratorPin7, HIGH);
    digitalWrite(vibratorPin8, HIGH);
    delay(1000);
    digitalWrite(vibratorPin, LOW);
    digitalWrite(vibratorPin1, LOW);
    digitalWrite(vibratorPin2, LOW);
    digitalWrite(vibratorPin3, LOW);
    digitalWrite(vibratorPin4, LOW);
    digitalWrite(vibratorPin5, LOW);
    digitalWrite(vibratorPin6, LOW);
    digitalWrite(vibratorPin7, LOW);
    digitalWrite(vibratorPin8, LOW);
    Serial.println("Bad posture detected, vibration triggered");
  }

  // Sending the retrieved data back to the client
  String response = "<h1>Received Posture: ";
  response += postureValue;
  response += "</h1><p><a href=\"/\"><button>Back</button></a></p>";
  server.send(200, "text/html", response);
}

