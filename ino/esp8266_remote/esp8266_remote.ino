// Embedded Linux (EMLI)
// University of Southern Denmark
// ESP8266 Wifi client - Webserver - User interface 
// Kjeld Jensen <kjen@sdu.dk> <kj@kjen.dk>
// 2023-04-18, KJ, First version
// inspired by https://docs.arduino.cc/tutorials/uno-wifi-rev2/uno-wifi-r2-hosting-a-webserver

// LED
#define PIN_LED_RED     14
#define PIN_LED_YELLOW  13
#define PIN_LED_GREEN   12
bool pumpAlarmOn = false;
bool plantAlarmOn = false;;

// button
#define PIN_BUTTON      4
#define DEBOUNCE_TIME 200 // milliseconds
volatile int button_a_count;
volatile unsigned long count_prev_time;

// Wifi
#include <ESP8266WiFi.h>
const char* WIFI_SSID = "EMLI_TEAM_19";
const char* WIFI_PASSWORD = "12341234";

// Static IP address
IPAddress IPaddress(192, 168, 10, 222);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);
//IPAddress primaryDNS(1, 1, 1, 1); 
//IPAddress secondaryDNS(8, 8, 8, 8); 

// Webserver
#define CLIENT_TIMEOUT 2000

#include <PubSubClient.h>
const char* mqtt_server = "192.168.10.1";

//WiFiServer server(80);
//WiFiClient client = server.available();
WiFiClient client;

unsigned long clientConnectTime = 0;
String currentLine = "";
char response_s[10];
char s[25];

PubSubClient mqttClient(client);

ICACHE_RAM_ATTR void button_a_isr()
{
  if (millis() - count_prev_time > DEBOUNCE_TIME)
  {
    count_prev_time = millis();
    mqttClient.publish("emli19/button", "1");
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "emli19/plant_water_alarm") {
    Serial.print("Changing output to ");
    if(messageTemp == "0"){
      Serial.println("off");
      plantAlarmOn = false;
    }
    else if(messageTemp == "1"){
      Serial.println("on");
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_YELLOW, LOW);
      plantAlarmOn = true;
    }
  }
    if (String(topic) == "emli19/pump_water_alarm") {
    Serial.print("Changing output to ");
    if(messageTemp == "0"){
      Serial.println("off");
      pumpAlarmOn = false;
    }
    else if(messageTemp == "1"){
      Serial.println("on");
      // For testing purposes, we use 1 as an alarm. However, in a running system a 0 will trigger an alarm.
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_YELLOW, LOW);
      pumpAlarmOn = true;
    }
  }
    if (String(topic) == "emli19/moisture") {
    Serial.print("Changing output to ");
    if(messageTemp.toInt() > 50){
      if (!pumpAlarmOn && !plantAlarmOn) {
        Serial.println("on");
        digitalWrite(PIN_LED_GREEN, HIGH);
        digitalWrite(PIN_LED_RED, LOW);
        digitalWrite(PIN_LED_YELLOW, LOW);
      }
    }
    else if(messageTemp.toInt() <= 50){
      if (!pumpAlarmOn && !plantAlarmOn) {
        Serial.println("off");
        digitalWrite(PIN_LED_RED, LOW);
        digitalWrite(PIN_LED_GREEN, LOW);
        digitalWrite(PIN_LED_YELLOW, HIGH);
      }
    }
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("ESP8266Client", "emli19","123")) {
      Serial.println("connected");
      // Subscribe
      mqttClient.subscribe("emli19/plant_water_alarm");
      mqttClient.subscribe("emli19/pump_water_alarm");
      mqttClient.subscribe("emli19/moisture");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  // serial
  Serial.begin(115200);
  delay(10);

  // LEDs
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_RED, LOW);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  digitalWrite(PIN_LED_YELLOW, LOW);
  pinMode(PIN_LED_GREEN, OUTPUT);
  digitalWrite(PIN_LED_GREEN, LOW);

  // button
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), button_a_isr, RISING);

  // set the ESP8266 to be a WiFi-client
  WiFi.mode(WIFI_STA); 
  
  // configure static IP address
  WiFi.config(IPaddress, gateway, subnet);
  //WiFi.config(IPaddress, gateway, subnet, primaryDNS, secondaryDNS);

  // connect to Wifi access point
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  // start webserver
  /*
  Serial.println("Starting webserver");
  Serial.println("");
  server.begin();
  */

  // Start MQTT Client
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
}

void loop()
{
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  
  /*
  client = server.available();
  if (client) {
    Serial.println("New client");
    currentLine = "";
    clientConnectTime = millis();
    while (client.connected() && millis() - clientConnectTime < CLIENT_TIMEOUT) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c == '\n') {
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            Serial.println("Sending response");
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type: text/html");
            sprintf (s, "Content-Length: %d",strlen(response_s));
            client.println(s);
            client.println();
            //client.println("Connection: close");
            client.println(response_s);
            client.println();
            response_s[0] = 0;
          } else {
            
            if (currentLine.startsWith("GET /led/red/on")) {
              Serial.println("Red LED on");
              digitalWrite(PIN_LED_RED, HIGH);
            } else if (currentLine.startsWith("GET /led/red/off")) {
              Serial.println("Red LED off");
              digitalWrite(PIN_LED_RED, LOW);
    
            } else if (currentLine.startsWith("GET /led/yellow/on")) {
              Serial.println("Yellow LED on");
              digitalWrite(PIN_LED_YELLOW, HIGH);
            } else if (currentLine.startsWith("GET /led/yellow/off")) {
              Serial.println("Yellow LED off");
              digitalWrite(PIN_LED_YELLOW, LOW);
    
            } else if (currentLine.startsWith("GET /led/green/on")) {
              Serial.println("Green LED on");
              digitalWrite(PIN_LED_GREEN, HIGH);
            } else if (currentLine.startsWith("GET /led/green/off")) {
              Serial.println("Green LED off");
              digitalWrite(PIN_LED_GREEN, LOW);

            } else if (currentLine.startsWith("GET /button/a/count")) {
              Serial.println("Return button count");
              sprintf (response_s, "%d", button_a_count);
              button_a_count = 0;
            }        

            currentLine = "";
          }          
        } else if (c != '\r') {
          currentLine += c;          
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    */ 
  }
