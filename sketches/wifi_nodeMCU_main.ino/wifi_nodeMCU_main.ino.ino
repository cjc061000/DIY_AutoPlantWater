#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

// --- Wifi credentials ----
const char* ssid = "ASUSHub24_2GEXT";
const char* password = "4thDimension";

// ---- POST http host and fingerprint -----
const char* resource = "/workflows/cbfa82c52742455388036835ecabf199/triggers/manual/paths/invoke?api-version=2016-10-01&sp=/triggers/manual/run&sv=1.0&sig=WL0LopWYLwt2-JA9e2uxKx1Uxg1spMEZfPM3h-a__hY";
const char* host = "prod-15.centralus.logic.azure.com";
int Sample;
const char fingerprint[] PROGMEM = "91 37 16 D9 A3 1A 86 3A DC 7E 60 BE 24 5A D8 0B 92 C0 6A 14";
const int httpsPort = 443;
WiFiClient client;

// ---- Pin Declaration -----
int sensorDigitalPin = D8;
int sensorAnalogPin = A0;

// ---- Logic global variables ----
int readDelay = 2000;
bool enableSendData = true;
bool isMessageSent = false;
int hysteresis = 100;
int threshold = 950;
bool isPlantDry = false;
int counter = 0;
int loopsToSkip = 3;

// ------- SETUP -------------
void setup(void){

  pinMode(sensorDigitalPin, INPUT);
  pinMode(sensorAnalogPin, INPUT);
   
  delay(readDelay);
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

// ***********************************
// ************ Main Loop ************
// ***********************************
void loop(void){
  int analogValue = analogRead(sensorAnalogPin);
  Serial.println("pin values read:");
  Serial.println(digitalRead(sensorDigitalPin));
  Serial.println(analogValue);
  
  // if we already skipped the couple initial loops
  if(counter >= loopsToSkip){
    // if value is above threshold, plants are dry
    if( (analogValue > threshold) ){
      isPlantDry = true;
      // if we already sent a message, dont spam email
      if(!isMessageSent){
        SendHttpValue(analogValue);
      }
    }
    if(analogValue <= (threshold-hysteresis) ){
      resetFlags();
    }
    
  }
  
  delay(readDelay);
  increaseCounter();
}

// ***********************************
// ********* AUX Functions ***********************
// ***********************************
void SendHttpValue(int analogValue){
  WiFiClientSecure httpsClient;    //Declare object of class HTTPClient
  Serial.println("Sending value to the internet");
  if(enableSendData){
    httpsClient.setFingerprint(fingerprint);
    httpsClient.setTimeout(15000); // 15 Seconds
    delay(1000);
  
    Serial.print("HTTPS Connecting");
    int r=0; //retry counter
    while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
        delay(100);
        Serial.print(".");
        r++;
    }
    if(r==30) {
      Serial.println("Connection failed");
    }
    else {
      Serial.println("Connected to web");
    }

    String postStr = "";
    postStr +="{\"humidity\":" + String(analogValue) + "}";
    
    Serial.println(String("POST ") + resource + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Content-Type: application/json"+ "\r\n" +
                   "Content-Length: 13" + "\r\n\r\n" +
                   postStr +
                   "Connection: close\r\n\r\n");
                 
    httpsClient.print(String("POST ") + resource + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Content-Type: application/json"+ "\r\n" +
                 "Content-Length: 13" + "\r\n\r\n" +
                 postStr +
                 "Connection: close\r\n\r\n");
  
    while (httpsClient.connected()) {
      String line = httpsClient.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }

    Serial.println(postStr);
    Serial.println("reply was:");
    Serial.println("==========");
    String line;
    while(httpsClient.available()){        
      line = httpsClient.readStringUntil('\n');  //Read Line by Line
      Serial.println(line); //Print response
    }
    Serial.println("==========");
    Serial.println("closing connection");
  }
  isMessageSent = true;
}

void increaseCounter(){
  if(counter < loopsToSkip){
    counter++;
  }
}

void resetFlags(){
  isMessageSent = false;
  isPlantDry = false;
}
