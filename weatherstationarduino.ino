#include <SoftwareSerial.h>
SoftwareSerial espSerial = SoftwareSerial(2, 3);

#include <DHT.h> 
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

String apiKey = "";

String ssid = "";
String password = "";

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

    String getStr = "GET /update?api_key="; // prepare GET string
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
        // alert user
        Serial.println("AT+CIPCLOSE");
        return false;
    }
    return true;
}

void setup() {
    Serial.begin(9600);

    dht.begin();

    espSerial.begin(9600);
    espSerial.println("AT+CWMODE=1");
    showResponse(1000);

    espSerial.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"");
    showResponse(5000);

    Serial.println("Setup completed");

    time1 = millis();
}

void loop() {

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
        Serial.println("Failed to read from DHT");
    } else {
        Serial.println("Temperature= " + String(t) + " *C");
        Serial.println("Humidity= " + String(h) + " %");
    }

    time2 = millis();

    if (time2 > time1 + 15000) {
        thingSpeakWrite(t, h);
        time1 = millis();
    }
    delay(500);
}
