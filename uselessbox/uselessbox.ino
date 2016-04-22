//We always have to include the library
#include "LedControl.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ascii_4x5.h"
#include "passwords.h"

// Stored in passwords.h
//#define wifi_ssid "YOUR WIFI SSID"
//#define wifi_password "WIFI PASSWORD"
//
//#define mqtt_server "192.168.0.23"
//#define mqtt_user ""
//#define mqtt_password ""

enum characters {A,r,d,u,i,n,o};

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 13 is connected to the DataIn
 pin 14 is connected to the CLK
 pin 12 is connected to LOAD
 We have 6 MAX72XX.
 */

LedControl lc = LedControl(13, 14, 12, 6);

/* we always wait a bit between updates of the display */
unsigned long delaytime = 100;
uint32_t buf[16];

WiFiClient espClient;
PubSubClient client(espClient);

const int motorAPWM = 15;
const int motorB    = 11;
const int switchPin = 9;


void setup() {
  Serial.begin(9600);
  Serial.println("Hello");
  wakeMAX72XX();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode (motorAPWM, OUTPUT);
  pinMode (motorB, OUTPUT);
  pinMode (switchPin, INPUT_PULLUP);
  attachInterrupt (switchPin, switchChange, CHANGE);
}


void switchChange(){
  // do switchy stuff here
  
}

void loop() {
    if (!client.connected()) {
    reconnect();
  }
  client.loop();
  writeScreen();
  delay(delaytime);
  for (int i = 0; i < 24; i++) {
    memset(buf, 0x0, 64);
     putSprite(i, 9, 4, 5,  r);
     putChar(i,0,'A');
     writeScreen();
    delay(delaytime);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
 Serial.print("Message arrived [");
 Serial.print(topic);
 Serial.print("] ");
  if (strcmp(topic, "uselessBox") == 0){
    for (int i=0;i<length;i++) {
      putChar(i*6, 0, payload[i]);
    }  
  }
 
 for (int i=0;i<length;i++) {
  char receivedChar = (char)payload[i];
  Serial.print(receivedChar);
  Serial.println();
}
}

void wakeMAX72XX(){
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  for (int i = 0; i < 6; i++) {
    lc.shutdown(i, false);
    /* Set the brightness to a medium values */
    lc.setIntensity(i, 8);
    /* and clear the display */
    lc.clearDisplay(i);
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// This function works for a single sprite so far WIP
void putSprite(int x, int y, int width, int height,  int sp) {
	for (int i = 0; i < height; i++) {
		buf[i + y] = (uint32_t(sprite[sp][i])) << (23 - x - width);
	}
}

// This function works for a single sprite so far WIP
void putChar(int x, int y, char sp) {
  int charNo = sp - 32;  // starting point from ascii array
  int width = arial_8ptDescriptors[charNo][0];
  int arrayPos = arial_8ptDescriptors[charNo][1];
  for (int i = 0; i < 8; i++) {  // add 8 lines into the buffer
    if (width == 1){
        buf[i + y] ^= (uint32_t(arial_8ptBitmaps[charNo + i])) << (23 - x - width);
    }
  }
}

// Write the contents of the buffer.... 
void writeScreen() {
	for (int display = 0; display < 3; display++) {						// Top Row
		for (int row = 0; row < 8; row++) {								// Cycle through row
			lc.setRow(2 - display, row, buf[row] >> (display * 8));		// set leds on top 3 MAX7219s
			lc.setRow(5 - display, row, buf[row + 8] >> (display * 8)); // set leds on bot 3 MAX7219s
		}
	}
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("uselessBox");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


