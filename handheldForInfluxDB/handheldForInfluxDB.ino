/* This code is FSK18 (contains "shit")*/

#include <ESP8266WiFi.h>
#include "SSD1306.h"

#define DEBUG 1
#define flipDisplay true
#define btn 0 //GPIO 0 = FLASH BUTTON

const char* ssid     = <Syntax error because of missing ssid>;
const char* password = <Syntax error because of missing pw>;

/* change the Port and IP of the influxdb connector */               
const int port = 8086;
const byte host[] = {192, 168, 178, 55};

/* The diferent queries for the timebases */
const String url_48h = "/query?db=HP_Baxxter&epoch=h&q=SELECT%20count%28%22value%22%29%20FROM%20%22powertick%22%20WHERE%20time%20%3E%20now%28%29%20-%2048h%20GROUP%20BY%20time%2824m%29%20fill%280%29";
const String url_24h = "/query?db=HP_Baxxter&epoch=h&q=SELECT%20count%28%22value%22%29%20FROM%20%22powertick%22%20WHERE%20time%20%3E%20now%28%29%20-%2024h%20GROUP%20BY%20time%2812m%29%20fill%280%29";
const String url_120min = "/query?db=HP_Baxxter&epoch=h&q=SELECT%20count%28%22value%22%29%20FROM%20%22powertick%22%20WHERE%20time%20%3E%20now%28%29%20-%20120m%20GROUP%20BY%20time%281m%29%20fill%280%29";
const String url_60min = "/query?db=HP_Baxxter&epoch=h&q=SELECT%20count%28%22value%22%29%20FROM%20%22powertick%22%20WHERE%20time%20%3E%20now%28%29%20-%2060m%20GROUP%20BY%20time%2830s%29%20fill%280%29";
const String url_30min = "/query?db=HP_Baxxter&epoch=h&q=SELECT%20count%28%22value%22%29%20FROM%20%22powertick%22%20WHERE%20time%20%3E%20now%28%29%20-%2030m%20GROUP%20BY%20time%2815s%29%20fill%280%29";

/* all teh other shit */
String line;

/* needed for the updating process */
unsigned long prevTime = 0;
unsigned long whait_s = 5000;
unsigned long curTime = 0;

String printline = "";
int request_mode = 0;
int val[128];
int sum = 0;
double multiplicator = 0.0;
unsigned long maxVal = 0;
bool canBtnPress =true;
bool update_display =true ; 
bool update_data =true ; 

extern "C" {
  #include "user_interface.h"
}

//===== SETTINGS =====//
//create display(Adr, SDA-pin, SCL-pin)
SSD1306 display(0x3c, 5, 4); //GPIO 5 = D1, GPIO 4 = D2


void setupWiFi(){
  line.reserve(512);
   // WiFi.mode(WIFI_STA);
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


void setup(){
    // Start up the Serial debugging
    #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("Setup");
    #endif

    //start up display
    display.init();
    if(flipDisplay) display.flipScreenVertically();

    // show start screen 
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Charge-");
    display.drawString(0, 16, "Monitor");
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "Copyright (c) 2017");
    display.drawString(0, 50, "Odon4t4");
    display.display();
    delay(2500);

    // Set up button mode
    pinMode(btn, INPUT);

    // Set up WiFi 
    setupWiFi();
}

/* This funktion is needed for the visualisation of the bars */
void getMultiplicator(){
    maxVal = 1;
    for(int i=0;i<128;i++){
      if(val[i] > maxVal) maxVal = val[i];
    }
    if(maxVal > 53) multiplicator = (double)53/(double)maxVal;
    else multiplicator = 1;
}

void loop() {
    curTime = millis();
      /* If the button was pressed we will change the mode and inform the user that something is going on */
      if(digitalRead(btn) == LOW){
        canBtnPress = false;
      }else if(!canBtnPress){
        canBtnPress = true;
        request_mode ++;
        if (request_mode==5) request_mode =0;
        update_data = true;
        display.clear();
        display.drawString(3, 0, "Loading DATA!!!");
        display.display(); 
     }
     /*Update the Dataset only every 5s or so */
     if(curTime - prevTime >= whait_s){  
        prevTime = curTime;
        update_data =true;
     }
     if(update_data){  //Update the Dataset only every 5s or so 
        update_data =false;
    
        /* Use WiFiClient class to create TCP connections*/
        WiFiClient client;
        if (!client.connect(host,8086)) {
            #ifdef DEBUG
            Serial.println("connection failed");
            #endif
            return;
        }

        /* Based on the request_mode we will send the request to the serve AND we will set the Update time*/
        if(request_mode == 0){
              client.print(String("GET ") + url_48h + " HTTP/1.1\r\n" +
                           "Host: " + "192.168.178.55:8086" + "\r\n" + 
                           "Connection: close\r\n\r\n");   
                           printline = "48h";
                           whait_s = 24*60*1000;
        }else if(request_mode == 1){
              // This will send the request to the server
              client.print(String("GET ") + url_24h + " HTTP/1.1\r\n" +
                           "Host: " + "192.168.178.55:8086" + "\r\n" + 
                           "Connection: close\r\n\r\n");
                           printline = "24h";
                           whait_s = 12*60*1000;
        }else if(request_mode == 2){
              // This will send the request to the server
              client.print(String("GET ") + url_120min + " HTTP/1.1\r\n" +
                           "Host: " + "192.168.178.55:8086" + "\r\n" + 
                           "Connection: close\r\n\r\n");
                           printline = "120min"; 
                           whait_s = 60*1000;           
        }else if(request_mode == 3){
              // This will send the request to the server
              client.print(String("GET ") + url_60min + " HTTP/1.1\r\n" +
                           "Host: " + "192.168.178.55:8086" + "\r\n" + 
                           "Connection: close\r\n\r\n");
                           printline = "60min"; 
                           whait_s = 30*1000;                      
        }else if(request_mode == 4){
              // This will send the request to the server
              client.print(String("GET ") + url_30min + " HTTP/1.1\r\n" +
                           "Host: " + "192.168.178.55:8086" + "\r\n" + 
                           "Connection: close\r\n\r\n");
                           printline = "30min";            
                           whait_s = 15*1000;           
        }
        delay(10);
      
        // Read all the lines of the reply from server
        /* the respond looks like:
              {"results":[{"statement_id":0,"series":[{"name":"powertick","columns":["time","count"],"values":[[416840,0],[416841,0],[416841,0],[416842,0],[416842,0],[416842,0],[416843,0],[416843,0],[416844,0],[416844,0],[416844,0],[416845,0],[416845,0],[416846,0],[416846,254],[416846,365],[416847,357],[416847,257],[416848,121],[416848,62],[416848,58],[416849,47],[416849,44],[416850,41],[416850,25],[416850,4],[416851,0],[416851,0],[416852,0],[416852,0],[416852,0],[416853,0],[416853,0],[416854,0],[416854,0],[416854,0],[416855,0],[416855,0],[416856,0],[416856,0],[416856,0],[416857,0],[416857,0],[416858,0],[416858,0],[416858,0],[416859,0],[416859,0],[416860,0],[416860,0],[416860,0],[416861,0],[416861,0],[416862,0],[416862,0],[416862,0],[416863,0],[416863,0],[416864,0],[416864,0],[416864,0],[416865,0],[416865,0],[416866,0],[416866,0],[416866,0],[416867,0],[416867,0],[416868,0],[416868,0],[416868,0],[416869,0],[416869,0],[416870,0],[416870,0],[416870,0],[416871,0],[416871,0],[416872,0],[416872,0],[416872,0],[416873,0],[416873,0],[416874,0],[416874,0],[416874,0],[416875,0],[416875,0],[416876,0],[416876,0],[416876,0],[416877,0],[416877,0],[416878,0],[416878,0],[416878,0],[416879,0],[416879,0],[416880,0],[416880,0],[416880,0],[416881,0],[416881,0],[416882,0],[416882,0],[416882,0],[416883,0],[416883,0],[416884,0],[416884,0],[416884,0],[416885,0],[416885,0],[416886,0],[416886,0],[416886,0],[416887,0],[416887,0],[416888,0],[416888,0],[416888,0]]}]}]}
          
           or if there are no datapoints:
              {"results":[{"statement_id":0}]}

        */
        while(client.available()){
            line = client.readStringUntil('\r');
            /* If there is at least a "result" in the responce we can do something*/
            if (line.substring(3,10) == "results") { 
                sum = 0;
                /* We will get "{"results":[{"statement_id":0}]}" if there are no datapoints */
                if (line.substring(30,33) == "}]}") {
                    for(int i =0 ; i<128 ; i++){
                      val[i] = 1;
                      sun=0;
                    }
                } else if (line.length() > 97){
                    line.remove(0,97); // all teh junk in front can be removed
                    int i=0;
                    int comma = 0;
                    int bracket = 0;
                    while(line.length() > 7){
                      comma  = line.indexOf(',');
                      bracket = line.indexOf(']');
                      if(i < 128){
                        val[i]=(line.substring(comma+1,bracket).toInt()+1); // get all the values out of the message
                        sum += val[i];
                        val[i] = val[i] * 100; // only for visaulisation the getMultiplicator methode will handle the rest
                        i++;
                      }
                      line.remove(0,bracket+2);
                    } 
                   /* There are some open fields... fill them */
                    for(; i < 128 ; i++){
                      val[i] = 1;
                    }
                }
                     
            }
        }
        update_display = true;

     }

    if (update_display){
          update_display = false;
          /*draw all the shit*/
          getMultiplicator();
      
          display.clear();
          for(int i=0;i<128;i++) display.drawLine(i, 64, i, 64-val[i]*multiplicator);
          display.drawString(0, 0, printline  + ":" + (String)(double(sum)/1000) + "kW");
          display.display(); 
    }
} 
