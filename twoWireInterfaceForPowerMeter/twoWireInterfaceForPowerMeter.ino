#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define S0 14 

#define DEBUG 1

const char* ssid     = <Syntax error because of missing ssid>;
const char* password = <Syntax error because of missing pw>;

int port = 8089;
byte host[] = {192, 168, 178, 55};

WiFiUDP udp;

void setup(){
    #ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Setup");
    #endif
    
    pinMode(S0, INPUT);
    attachInterrupt(S0, highInterrupt, RISING);
    setupWiFi();
    

}

void highInterrupt(){
    detachInterrupt(S0);
    attachInterrupt(S0, lowInterrupt, FALLING);
    
    #ifdef DEBUG
    Serial.println("high interrupt");
    #endif
    
}

void lowInterrupt(){
    detachInterrupt(S0);
    attachInterrupt(S0,  highInterrupt, RISING);
    
    #ifdef DEBUG
    Serial.println("low interrupt");
    #endif
    sendMessageToServer();
}


void setupWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifdef DEBUG
    Serial.print(".");
    #endif
  }

  #ifdef DEBUG
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  #endif
}


void sendMessageToServer(){
   String content = "powertick value=1";
   udp.beginPacket(host, port);
   udp.print(content);
   udp.endPacket();
}

void loop(){
}
