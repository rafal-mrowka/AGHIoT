
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* ssid = "iotagh";
const char* password = "IoTiISW123";

const char* host = "axurg0y0vtb2z-ats.iot.eu-west-1.amazonaws.com";

WiFiClientSecure wifiClient;

// xxxxxxxxxx-certificate.pem.crt
const char* certificate_pem_crt = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAKV7h05jxqk5RUbeGd7TDTdmQYEEMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMjA5MTgxMTA2
MDVaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDkw8gl8JOEKMp8jJMI
sg5FTV0oblJ8xJcxKKOkun7ZPT2MhG67tBi7nhY4DrJlMecnENl3jxC1g6r4uZNt
zvLq7e6LAyGT4WiaYyJabDqdQrJ5sNrMoMrntW6d6YaKrWuvvMQFk4BsrnRsW3yt
AgObs+FlWIpPJdr9kuHaux+QZEdgB1x5P0d5vmma2N+vwHOVZv/5/c7AmmuTr6o/
wLcNJGL6kieVDnIq5Cs7RlHMtF2TZafJdWyet68VxerOU1310ziYceRO7dL939nU
/2TotwnVnhsduVSHDh+8qaKE7hZXTddBmuIgKMMFyuEtbYAr99i2J7lcs7y1lUWm
bEpZAgMBAAGjYDBeMB8GA1UdIwQYMBaAFHhpIp5VRlqXCCKCD3GhFDy2pcn5MB0G
A1UdDgQWBBTOC3vRZIAtq+UhztNgqBetuPxa4DAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEABkp6fOTkcyy9+yXvfFvRLYS6
oCp1iMip0ugk8JNt/eRLf5nkDBoWtD3jHYwDTyEsu9Wvr8EwAXRRDTFf8nYu5uWE
w7uDffvxTinXCXrT2n5W8xCAzizt1ruSpNOYtpDXO5oUJ+jPPZNg6/IIlA2Q65ca
JM8MlML5F1e/+IeSMLhBwEnRmlRlbzkcPVJerK9J8TYXfVDWS7todybu4iEZXf2O
AG8ThJbYsLzVZNGqD1+400YzqHeLjyXgLEGLK8wg2ZoUlCex4xFl6BXEw2hfupnw
C1mbuxxGywe+w5oN2UN/5AnL2CkpGmbzj2nraX+x7K+fF1Ih2XxRmz4YoM2A5Q==
-----END CERTIFICATE-----
)EOF";

// xxxxxxxxxx-private.pem.key
const char* private_pem_key = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEA5MPIJfCThCjKfIyTCLIORU1dKG5SfMSXMSijpLp+2T09jIRu
u7QYu54WOA6yZTHnJxDZd48QtYOq+LmTbc7y6u3uiwMhk+FommMiWmw6nUKyebDa
zKDK57VunemGiq1rr7zEBZOAbK50bFt8rQIDm7PhZViKTyXa/ZLh2rsfkGRHYAdc
eT9Heb5pmtjfr8BzlWb/+f3OwJprk6+qP8C3DSRi+pInlQ5yKuQrO0ZRzLRdk2Wn
yXVsnrevFcXqzlNd9dM4mHHkTu3S/d/Z1P9k6LcJ1Z4bHblUhw4fvKmihO4WV03X
QZriICjDBcrhLW2AK/fYtie5XLO8tZVFpmxKWQIDAQABAoIBAQDHrlElNMI+yLA+
WSH6pBkqyuW25d1ghOUIBtYYcqVIkFkKL4rMkt5H3iy8z69N/2oqQl3gK2PFXOG+
65VRcyO8huUCOEB16Xo50LBqxy++lzpcWFAT74JNEKQ2eGF06P0nT5i22VJvCzQa
ZR0Fks19el0S7l8DySJG1pAIMe/SQt88OMGaespXCs7+BoVt1zckWvmQsXu14npP
JgGDLgqBXtMhS2HaSEcOMig4Di/wXX5lvyy7RI1Zp0GL3zJ/qkoi6FolGEAWHQGi
TaCyKH0B5GaKGKYcdVeJIyo3NN3QwI9VQyN890Tiv6bB6dr5/uAvqqkjv0ZcH8Ff
r2VwvhoRAoGBAPUsIkAbLSbnMgy/CsPA5CeMlvgugJhzaF0PMWr4xZfu4fRrd0a2
NJWyasWks8XboyVTFSzqiRfQO/vC6F7mIfD+M2ucISlkqqy/RwBSlDMNyv5swjFp
GIbKafG3WEH++6wiRyeZKhEE0owbw2t8F8fAJLlzZRO75cFXK5JVubKtAoGBAO7e
JaAHgIeSp7APn/bHZDYbsnj49Myen8yuWiXV1pDLL6XCXYfGzjhKDX2VOBmJcZDh
Upg1MYp51nttv6oVXI2rsURV3Y/QPlOKWdggqcB8IwvndIp3HrBueuCPHIDUt2/M
IHthBeuDtBQClNog+ODjLDbETyJlxcpJvSJfvZfdAoGAWIAS6QMmXFmudm1rzFfP
w/r68CGRJ/bY4GbAlvWwFfUUOW+lga/58WOTKT8X6b8r+Cgyh6Et1ZvEW0/zI3Z9
IAoSs7CLQ+7jsTHmxlYIyxkagpbtzno6cUWV0rw1LrWFUZCaENA4ICaWfh7zftQv
nVpdp1fhT6tO92ipAMzTGI0CgYB96ZfHvWyhCkteYT4fJYfLhdoULpdL7SmrCRT3
8LqD5LcfF0+aLM+zmEV4N1o1C+BTUJLrLM8KgaLvuTBZBZ2eCC0PKnv6PPk9rCYj
6UPR9R28PquN+bCxq+sVyBpsRrTTN7S/lKs8NFaOXbX9LXhDgzxhOiXEqgUS1r/e
CL21+QKBgQC/ASg1lSyAKJHzo4TA5IdOzhNqD8WLtZiSkLtbBQLGFQbcUJkCw34p
w+rBA6a8E0T254PK5ft1sIviHxTywBooYa4dXMEb1ogEfkTGV5nTOHnl/MlEwIkx
oLo//6y7Cj1jveP9smROT6HhFwA8f507O1V9gK566ojDsvfsSFq2/w==
-----END RSA PRIVATE KEY-----
)EOF";

// The AWS IoT CA Certificate from: 
// https://docs.aws.amazon.com/iot/latest/developerguide/managing-device-certs.html#server-authentication

const char* rootCA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";


void msgReceived(char* topic, byte* payload, unsigned int len);

PubSubClient pubSubClient(host, 8883, msgReceived, wifiClient); 

void setup() {
  Serial.begin(115200); delay(50); Serial.println();
  Serial.println("ESP32 MQTT AWS example");
  Serial.printf("SDK version: %s\n", ESP.getSdkVersion());

  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());

  wifiClient.setCACert(rootCA);
  wifiClient.setCertificate(certificate_pem_crt);
  wifiClient.setPrivateKey(private_pem_key);
}

unsigned long lastPublish;
int msgCount;

void loop() {

  wifiClient.setTimeout(15000); // 15 Seconds
  delay(1000);
  
  if (!pubSubClient.connected()) {
    Serial.print("PubSubClient connecting to: "); Serial.print(host);
    while (!pubSubClient.connected()) {
      Serial.print(".");
      pubSubClient.connect("ESP32thing");
      delay(1000);
    }
    Serial.println(" connected");
    pubSubClient.subscribe("read");
  }
  pubSubClient.loop();

  char sensorData[128]; 
  float temp = random(0,30); 
  
  sprintf(sensorData,  "{\"time\":%lu,\"temp\":%f}", millis() / 1000, temp); 

  if (millis() - lastPublish > 10000) {
  boolean rc = pubSubClient.publish("write", sensorData);
    Serial.print("Message published, rc="); Serial.print( (rc ? "OK: " : "FAILED: ") );
    Serial.println(sensorData);
    lastPublish = millis();  
  }
}

void msgReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on "); 
  Serial.print(topic); 
  Serial.print(": ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
