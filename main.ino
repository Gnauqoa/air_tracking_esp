#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

#define BLYNK_TEMPLATE_ID "TMPLxVBmc8hg"
#define BLYNK_TEMPLATE_NAME "esp8266"
#define BLYNK_AUTH_TOKEN "7LMSa_TpxedslVa958ZgCGKxN8RPK1B_"
#define BLYNK_PRINT Serial


#include <BlynkSimpleEsp8266.h>

const char ssid[] = "quang";
const char password[] = "quang123";
float value = 0;
float alert = 0;

byte num_sensor = 1;
byte sensor_ID[225];
double sensor_Val[225] = { 1 };

bool flag_updateValue = 0;
bool flag_sendSerial = 0;
bool flag_connected = 0;
bool flag_APreconnect = 0;
bool flag_update_alert = 0;
bool V20_output = 0;
bool V21_output = 0;
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
unsigned long timer_sendSerial = 0;
unsigned long time_sendSerial = 1000;
unsigned long timer_updateValue = 0;
unsigned long time_updateValue = 15000;

WiFiManager wifiManager;
BLYNK_WRITE(20) {
  V20_output = param.asInt();
}
BLYNK_WRITE(21) {
  V21_output = param.asInt();
}
void read_Serial();
bool check_Timer(unsigned long timer, unsigned long _time);
void timer_Control();
void APreconnect();
void send_Serial();
int check_ID(byte ID);
void check_Connect();
void sendDataToServer();
void setup() {
  Serial.begin(9600);
  wifiManager.setTimeout(100);
  if (!wifiManager.autoConnect("Air tracking")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}
void loop() {

  read_Serial();
  check_Connect();
  timer_Control();
  sendDataToServer();
}
void sendDataToServer() {
  for (int i = 0; i < num_sensor; ++i) {
    Blynk.virtualWrite(i, sensor_Val[i]);
  }
}
void timer_Control(void) {
  if (check_Timer(timer_sendSerial, time_sendSerial)) {
    flag_sendSerial = 1;
    timer_sendSerial = millis();
  }
  if (check_Timer(timer_updateValue, time_updateValue)) {
    flag_updateValue = 1;
    timer_updateValue = millis();
  }
}
bool check_Timer(unsigned long timer, unsigned long _time) {
  if (millis() >= timer) {
    if (millis() - timer >= _time) {
      return 1;
    }
  }
  if (millis() < timer) {
    if (60 - timer + millis() >= _time) {
      return 1;
    }
  }
  return 0;
}
void APreconnect(void) {
  if (flag_APreconnect) {
    if (!wifiManager.autoConnect("Air tracking")) {
      delay(3000);
      ESP.reset();
    }
    delay(5000);
  }
  flag_APreconnect = 0;
}
void send_Serial(void) {
  String data = "K-" + (String)flag_connected + '-' + (String)alert + '-' + (String)V20_output + '-' + (String)V21_output;
  Serial.println(data);
}
int check_ID(byte ID) {
  for (int i = 0; i < num_sensor; ++i) {
    if (ID == sensor_ID[i]) {
      return i;
    }
  }
  return -1;
}
void read_Serial(void) {
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    if (s.indexOf("Data-") != -1)
      if (s.indexOf("No Sensor") == -1) {
        byte fist = 5;
        byte last = s.indexOf("-", fist);
        num_sensor = (s.substring(fist, last)).toInt();
        fist = last + 1;
        last = s.indexOf("-", fist);
        byte sensor_get = (byte)(s.substring(fist, last)).toInt();
        fist = last + 1;
        last = s.indexOf("-", fist);
        byte sensorID_get = (s.substring(fist, last)).toInt();
        fist = last + 1;
        last = s.indexOf("-", fist);
        float sensorVal_get = (s.substring(fist, last)).toFloat();
        fist = last + 1;
        last = s.length();
        byte index = byte((s.substring(fist, last)).toInt());
        sensor_Val[index] = sensorVal_get;
        sensor_ID[index] = sensorID_get;
        send_Serial();
      }
    if (s.indexOf("L-") != -1) {
      byte fist = s.indexOf("-") + 1;
      byte last = s.length();
      alert = (s.substring(fist, last)).toFloat();
      flag_update_alert = 1;
    }
  }
}
void check_Connect(void) {
  if (WiFi.isConnected()) {
    flag_connected = 1;
    flag_APreconnect = 0;
  } else {
    flag_connected = 0;
    flag_APreconnect = 1;
  }
}