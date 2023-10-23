#include <WiFiClientSecure.h>

const char* ssid = "iotagh";
const char* password = "IoTiISW123";

const char* host = "192.168.1.175";
const int Port = 8080;

const char *uri = "/write";

WiFiClient wifiClient;

void setup() {
  Serial.begin(115200); delay(50); Serial.println();
  Serial.println("ESP32 HTTP example");
  Serial.printf("SDK version: %s\n", ESP.getSdkVersion());

  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());

}

void loop() {
  Serial.println(host);
  wifiClient.setTimeout(15000); // 15 Seconds
  delay(1000);
  
  Serial.print("HTTP Connecting to the host\n");
  int r=0; //retry counter
  while((!wifiClient.connect(host, Port)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }
  if(r==30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected");
  }

  char sensorData[256]; 
  float temp = random(0,30); 
  
  sprintf(sensorData,  "{\"time\":%lu,\"temp\":%f}", millis() / 1000, temp); 
                                                                                                     
  char postRequest[48];
  sprintf(postRequest, "POST %s HTTP/1.1", uri);
  
  Serial.println("Request is sending");
  
  wifiClient.println(postRequest);
  wifiClient.print("Host: ");
  wifiClient.println(host);
  wifiClient.println("Content-Type: application/json");
  wifiClient.print("Content-Length: "); 
  wifiClient.println(strlen(sensorData));
  wifiClient.println();
  wifiClient.println(sensorData);          

  Serial.println("Request sent");

  while(wifiClient.connected()) {
    String line = wifiClient.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r")
      break;
  }  
  Serial.println("BODY start");
  String line = wifiClient.readStringUntil('\n');
  Serial.println(line);
  Serial.println("BODY stop");

  delay(10000);                       
  wifiClient.stop();                    
}
