#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h> 

// --- OLED CONFIG ---
#define SCREEN_WIDTH 64 
#define SCREEN_HEIGHT 48
#define OLED_RESET -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- SERVO CONFIG ---
Servo statusServo;
const int servoPin = 12; 

// --- STATE TRACKING ---
enum SystemStatus { STATUS_OK, STATUS_WARNING, STATUS_CRITICAL, STATUS_NO_DATA, STATUS_INIT };
SystemStatus lastStatus = STATUS_INIT; 

// --- THRESHOLDS ---
float vibCritical = 50.0; 
float vibWarning = 38.0;  
float dryLimit = 15.0;    
unsigned long dataTimeout = 7000; 

// --- NETWORK CONFIG ---
const char* ssid = "Orange-c45e0";
const char* password = "48LW8yBb";
const char* mqttServer = "192.168.0.9"; 

float curDHT=0, curBMP=0, curHum=0;
unsigned long lastMsgTime = 0; 

WiFiClient espClient;
PubSubClient client(espClient);

// --- SEPARATE SERVO FUNCTION ---++
void triggerServo(int angle, String label) {
  Serial.print(">>> STATUS CHANGE: ");
  Serial.print(label);
  Serial.print(" | Moving to: ");
  Serial.println(angle);
  

  statusServo.attach(servoPin, 544, 2400); 
  
  statusServo.write(angle);
  delay(600);           // Slightly longer delay to ensure full travel
  statusServo.detach(); 
}

void updateDisplay(SystemStatus s) {
  display.clearDisplay();
  display.setTextSize(1); 
  display.setTextColor(WHITE);
  display.setCursor(0, 4); 

  switch(s) {
    case STATUS_NO_DATA:  display.print(">> NO DATA");  break;
    case STATUS_CRITICAL: display.print(">> CRITICAL"); break;
    case STATUS_WARNING:  display.print(">> WARNING");  break;
    case STATUS_OK:       display.print(">> SYS OK");   break;
    default:              display.print(">> BOOTING");  break;
  }

  display.drawFastHLine(0, 20, 64, WHITE);
  display.display();
}

void checkStatusChange() {
  SystemStatus currentStatus;

  if (millis() - lastMsgTime > dataTimeout) {
    currentStatus = STATUS_NO_DATA;
  } else {
    bool isCritical = (curBMP > vibCritical && curHum < dryLimit);
    bool isWarning = (curBMP > vibWarning || curDHT > 35.0);

    if (isCritical) currentStatus = STATUS_CRITICAL;
    else if (isWarning) currentStatus = STATUS_WARNING;
    else currentStatus = STATUS_OK;
  }

  if (currentStatus != lastStatus) {
    lastStatus = currentStatus;
    updateDisplay(currentStatus);

    if (currentStatus == STATUS_CRITICAL) triggerServo(180, "CRITICAL");
    else if (currentStatus == STATUS_WARNING) triggerServo(90, "WARNING");
    else if (currentStatus == STATUS_OK) triggerServo(0, "SYS OK");
    else if (currentStatus == STATUS_NO_DATA) triggerServo(90, "NO DATA");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  lastMsgTime = millis(); 
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  float val = msg.toFloat();

  if (strstr(topic, "dht_temp")) curDHT = val;
  else if (strstr(topic, "bmp_temp")) curBMP = val;
  else if (strstr(topic, "humidity")) curHum = val;

  checkStatusChange(); 
}

void setup() {
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  
  Wire.begin(21, 22);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { for(;;); }
  
  updateDisplay(STATUS_INIT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
  lastMsgTime = millis(); 
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32_Vibe_Stat_" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe("fabriek/machine/#");
    } else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 1000) {
    lastCheck = millis();
    checkStatusChange(); 
  }
}