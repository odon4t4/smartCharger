#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

/* Here is the S0 interface connected to*/
#define S0 14 

#define DEBUG 1

const char* ssid     = <Syntax error because of missing ssid>;
const char* password = <Syntax error because of missing pw>;

/* change the Port and IP of the influxdb connector */               
int port = 8089;
byte host[] = {192, 168, 178, 55};

WiFiUDP udp;

void setup(){
    #ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Setup");
    #endif

    /*Set the Pin As input and define a interrupt for a rising signal*/
    pinMode(S0, INPUT);
    attachInterrupt(S0, highInterrupt, RISING);
    setupWiFi();
}

/*handle the interrupt and define on for a Falling signal*/
void highInterrupt(){
    detachInterrupt(S0);
    attachInterrupt(S0, lowInterrupt, FALLING);
    
    #ifdef DEBUG
    Serial.println("high interrupt");
    #endif
    
}

/*handle the Interrupt and define again for a rising signal also we need to send a message.*/
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

/*send out a message to the influxdb which should be configured to fulfil the rest of the stuff*/
void sendMessageToServer(){
   String content = "powertick value=1";
   udp.beginPacket(host, port);
   udp.print(content);
   udp.endPacket();
}

void loop(){
}
