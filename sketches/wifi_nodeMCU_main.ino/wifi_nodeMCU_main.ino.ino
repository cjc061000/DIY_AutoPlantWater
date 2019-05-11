#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid = "ASUSHub24_2GEXT";
const char* password = "4thDimension";

const char* resource = "/workflows/3495f95b917a42e9a39e7959423e28dd/triggers/manual/paths/invoke?api-version=2016-10-01&sp=%2Ftriggers%2Fmanual%2Frun&sv=1.0&sig=nCoh7VcTBQrJ8ncKPKky5DupaQmB8d5zkuLhZmNCKe8";
const char* host = "prod-09.centralus.logic.azure.com";
int Sample;

WiFiClient client;

int sensorDigitalPin = D8;
int sensorAnalogPin = A0;

void setup(void){

  pinMode(sensorDigitalPin, INPUT);
  pinMode(sensorAnalogPin, INPUT);
   
  delay(1000);
  Serial.begin(115200);
  WiFi.begin(ssid, password); //begin WiFi connection
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void loop(void){
  int analogValue = analogRead(sensorAnalogPin);
  Serial.println(digitalRead(sensorDigitalPin));
  Serial.println(analogValue);
  delay(1000);
  if (client.connect(host,443))
  { 
    String postStr = "";
    postStr +="{\"mydata\":" + String(analogValue) + "\"}";

    Serial.println(postStr);

    client.print("POST ");
    client.print(resource);
    client.print(" HTTP/1.1\n");
    client.print("Host:");
    client.print(host); client.print("\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/json\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    while(client.available()){
      char c = client.read();
      Serial.print(c);
    }
    Serial.println("");
  }
  client.stop();
  Serial.println("Waiting…");
  delay(10000);
}
