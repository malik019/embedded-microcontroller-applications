#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <LiquidCrystal_I2C.h>
#include <Bonezegei_DHT11.h>

// WiFi & MQTT Config
const char* ssid = "Orange-c45e0";
const char* password = "48LW8yBb";
const char* mqttServer = "192.168.0.9"; 
const int mqttPort = 1883;
const char* mqttUser = "cherry"; 
const char* mqttPassword = "mqtt-test"; 
const char* clientID = "esp32-c6"; 

// Pin Config
#define SDA_PIN 4 
#define SCL_PIN 5 
#define DHT_PIN 9

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_BMP280 bmp; 
LiquidCrystal_I2C lcd(0x27, 16, 2);
Bonezegei_DHT11 dht(DHT_PIN);

unsigned long lastMsg = 0;
bool showDHT = false; // Toggle for LCD screens

void setup() {
  Serial.begin(115200);
  
  // 1. Start I2C & DHT
  Wire.begin(SDA_PIN, SCL_PIN);
  dht.begin();
  delay(2000); 

  // 2. Start LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("System Booting...");

  // 3. Start BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 Error!");
    lcd.setCursor(0,1);
    lcd.print("Sensor Error");
    while (1);
  }

  // 4. Start WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  lcd.clear();
  lcd.print("WiFi Connected");
  client.setServer(mqttServer, mqttPort);
  delay(1000);
}

void loop() {
  if (!client.connected()) {
    if (client.connect(clientID, mqttUser, mqttPassword)) {
      Serial.println("MQTT Connected");
    } else {
      delay(5000);
      return;
    }
  }
  client.loop(); 

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // Read BMP280
    float bmpTemp = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F;

    // Read DHT11
    float dhtTemp = 0;
    int humidity = 0;
    if (dht.getData()) {
      dhtTemp = dht.getTemperature();
      humidity = dht.getHumidity();
    }

    // --- LCD DISPLAY (Alternates every 2 seconds) ---
    lcd.clear();
    if (!showDHT) {
      // Screen 1: BMP280 Data
      lcd.setCursor(0, 0);
      lcd.print("BMP Temp: "); lcd.print(bmpTemp, 1); lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Pres: "); lcd.print(pressure, 0); lcd.print(" hPa");
      showDHT = true; 
    } else {
      // Screen 2: DHT11 Data
      lcd.setCursor(0, 0);
      lcd.print("DHT Temp: "); lcd.print(dhtTemp, 1); lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Humidity: "); lcd.print(humidity); lcd.print("%");
      showDHT = false;
    }

    // --- SEND MQTT ---
    client.publish("fabriek/machine/bmp_temp", String(bmpTemp).c_str());
    client.publish("fabriek/machine/pressure", String(pressure).c_str());
    client.publish("fabriek/machine/dht_temp", String(dhtTemp).c_str());
    client.publish("fabriek/machine/humidity", String(humidity).c_str());
    
    // Serial Monitor Backup
    Serial.print("BMP:"); Serial.print(bmpTemp);
    Serial.print(" | DHT:"); Serial.print(dhtTemp);
    Serial.print(" | Hum:"); Serial.println(humidity);
  }
}
