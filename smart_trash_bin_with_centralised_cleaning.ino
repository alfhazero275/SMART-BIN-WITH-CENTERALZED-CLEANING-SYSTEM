#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ================= WIFI =================
const char* ssid = "RedmiPower";
const char* password = "";

// ================= DUSTBIN ID =================
String dustbinID = "DB-01";

// ================= PINS =================

// Human Detection Sensor
#define TRIG1 5
#define ECHO1 18

// Waste Level Sensor
#define TRIG2 19
#define ECHO2 21

// LEDs
#define GREEN_LED 2
#define RED_LED 4

// Servo
#define SERVO_PIN 13

// ================= OBJECTS =================

WebServer server(80);
Servo myServo;

// ================= VARIABLES =================

int humanDistance = 0;
int wasteDistance = 0;

String dustbinStatus = "NORMAL";
String alertMessage = "NO ALERT";

bool alertSent = false;

// =================================================
// DISTANCE FUNCTION
// =================================================

int getDistance(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  int distance = duration * 0.034 / 2;

  if(distance <= 0 || distance > 400)
    distance = 400;

  return distance;
}

// =================================================
// WEB PAGE
// =================================================

void handleRoot()
{
  String page = "";

  page += "<html><head>";
  page += "<meta http-equiv='refresh' content='2'>";
  page += "<title>Smart Dustbin</title>";
  page += "</head><body style='font-family:Arial;text-align:center;'>";

  page += "<h1>SMART DUSTBIN</h1>";

  page += "<h2>Dustbin ID : ";
  page += dustbinID;
  page += "</h2>";

  page += "<h3>Human Distance : ";
  page += String(humanDistance);
  page += " cm</h3>";

  page += "<h3>Waste Distance : ";
  page += String(wasteDistance);
  page += " cm</h3>";

  page += "<h2>Status : ";
  page += dustbinStatus;
  page += "</h2>";

  page += "<h2>";
  page += alertMessage;
  page += "</h2>";

  page += "</body></html>";

  server.send(200, "text/html", page);
}

// =================================================
// SETUP
// =================================================

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  myServo.attach(SERVO_PIN);
  myServo.write(0);

  // WiFi
  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.begin();

  Serial.println("Web Server Started");
}

// =================================================
// LOOP
// =================================================

void loop()
{
  server.handleClient();

  // Human Detection
  humanDistance = getDistance(TRIG1, ECHO1);

  if(humanDistance < 20)
  {
    myServo.write(90);
    delay(3000);
    myServo.write(0);
  }

  // Waste Detection
  wasteDistance = getDistance(TRIG2, ECHO2);

  Serial.print("Waste Distance = ");
  Serial.println(wasteDistance);

  if(wasteDistance > 20)
  {
    dustbinStatus = "NORMAL";

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    alertMessage = "NO ALERT";

    alertSent = false;
  }
  else if(wasteDistance > 10)
  {
    dustbinStatus = "AVERAGE";

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    alertMessage = "BIN GETTING FULL";
  }
  else
  {
    dustbinStatus = "FULL";

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    if(!alertSent)
    {
      alertMessage =
      "ALERT SENT TO HEAD OFFICE - Dustbin ID: " + dustbinID;

      Serial.println("================================");
      Serial.println("ALERT SENT TO HEAD OFFICE");
      Serial.println("Dustbin ID: " + dustbinID);
      Serial.println("STATUS: FULL");
      Serial.println("Cleaner Required");
      Serial.println("================================");

      alertSent = true;
    }
  }

  delay(500);
}