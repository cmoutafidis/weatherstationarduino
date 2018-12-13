#include <SFE_BMP180.h>
#include <Wire.h>
#define ALTITUDE 1655.0

#include <SoftwareSerial.h>
SoftwareSerial espSerial = SoftwareSerial(2, 3);

#include <DHT.h> 
#define DHTPIN 5
#define DHTPIN2 6
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

SFE_BMP180 pressure;

String apiKey = "";

String ssid = "";
String password = "";

const int sensorMin = 0;
const int sensorMax = 1024;

long time1;
long time2;

void showResponse(int waitTime) {
    long t = millis();
    char c;
    while (t + waitTime > millis()) {
        if (espSerial.available()) {
            c = espSerial.read();
            Serial.print(c);
        }
    }
}

boolean thingSpeakWrite(float value1, float value2) {
    String cmd = "AT+CIPSTART=\"TCP\",\"";
    cmd += "184.106.153.149";
    cmd += "\",80";
    espSerial.println(cmd);
    Serial.println(cmd);
    if (espSerial.find("Error")) {
        Serial.println("AT+CIPSTART error");
        return false;
    }

    String getStr = "GET /update?api_key=";
    getStr += apiKey;

    getStr += "&field1=";
    getStr += String(value1);
    getStr += "&field2=";
    getStr += String(value2);
    getStr += "\r\n\r\n";

    cmd = "AT+CIPSEND=";
    cmd += String(getStr.length());
    espSerial.println(cmd);
    Serial.println(cmd);

    delay(100);
    if (espSerial.find(">")) {
        espSerial.print(getStr);
        Serial.print(getStr);
    } else {
        espSerial.println("AT+CIPCLOSE");
        Serial.println("AT+CIPCLOSE");
        return false;
    }
    return true;
}

void setup() {
    Serial.begin(9600);
    if (pressure.begin()){
      Serial.println("BMP180 init success");
    }
    else{
      // Oops, something went wrong, this is usually a connection problem,
      // see the comments at the top of this sketch for the proper connections.
  
      Serial.println("BMP180 init fail\n\n");
      while(1); // Pause forever.
    }

    dht.begin();
    dht2.begin();

    espSerial.begin(9600);
    espSerial.println("AT+CWMODE=1");
    showResponse(1000);

    espSerial.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"");
    showResponse(5000);

    Serial.println("Setup completed");

    time1 = 0;
}

void loop() {
    int sensorReading = analogRead(A0);
    int range = map(sensorReading, sensorMin, sensorMax, 0, 3);
    switch (range) {
    case 0:    // Sensor getting wet
        Serial.println("Flood");
        break;
    case 1:    // Sensor getting wet
        Serial.println("Rain Warning");
        break;
    case 2:    // Sensor dry - To shut this up delete the " Serial.println("Not Raining"); " below.
        Serial.println("Not Raining");
        break;
    }
  
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
        Serial.println("Failed to read from DHT");
    } else {
        Serial.println("Temperature= " + String(t) + " *C");
        Serial.println("Humidity= " + String(h) + " %");
    }

    float t2 = dht.readTemperature();
    float h2 = dht.readHumidity();
    if (isnan(t2) || isnan(h2)) {
        Serial.println("Failed to read from DHT2");
    } else {
        Serial.println("Temperature2= " + String(t2) + " *C");
        Serial.println("Humidity2= " + String(h2) + " %");
    }

    time2 = millis();

    if (time2 > time1 + 15000) {
        thingSpeakWrite(t, h);
        time1 = millis();
    }
    delay(500);
}
