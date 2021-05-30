/*
Arduino Nano RP2040 template for mqtt projects
The intention is to create a rock solid if is possible.
Created from different sources.

May 30 2021
jnogues@gmail.com
@rprimTech
 
This example code is in the public domain

*/

#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "mbed.h"
#include <rtos.h>
using namespace rtos;
using namespace mbed;

#include <SPI.h>
#include <WiFiNINA.h>
NinaPin ledR = LEDR;
NinaPin ledG = LEDG;
NinaPin ledB = LEDB;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "yourSSID";        // your network SSID (name)
char pass[] = "yourPASS";    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the WiFi radio's status

Thread th1;
Thread th2;
Thread th3;
Ticker tck1;

volatile bool ticTac=0;
volatile bool wifiOK = 0;
volatile bool mqttOK = 0;

WiFiClient mqttClient;
PubSubClient client(mqttClient);
const char* mqtt_server = "xx.xx.xx.xx";
unsigned long lastMsg = 0;

StaticJsonDocument<512> doc;

const uint32_t TIMEOUT_MS = 10000;//don't work the maximum is 8.3 seconds
const uint32_t TIME_BETWEEN_RESETS_MS = 3600000; //1 hour


void setup() 
{
  
  Serial.begin(115200);
  while (!Serial) {}; // wait for serial port to connect. Needed for native USB port only
  Serial.println("[BOOT] .............Starting............");
  pinMode(13,OUTPUT);
  digitalWrite(13, LOW);
  // Setup the 3 pins as OUTPUT
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  digitalWrite(ledR, LOW);
  digitalWrite(ledG, LOW);
  digitalWrite(ledB, LOW);

  Serial.print("[WTHD] Max Timeout Whatchdog = ");
  Serial.print(Watchdog::get_instance().get_max_timeout());
  Serial.println(" uS");
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Watchdog &watchdog = Watchdog::get_instance();
  watchdog.start();
  Watchdog::get_instance().kick();//clear whatchdog
   
  th1.start(task_blinkLed13);
  th2.start(task_reconnectingWIFI);
  th3.start(task_softwareReset);
  tck1.attach(&flipp, 1.0);//Using Ticker
}

void loop() 
{
    ThisThread::sleep_for(10000);//like delay(10000)
    publishState();
    Watchdog::get_instance().kick();
    
}


//************************** mbed OS Tasks *************************
void task_softwareReset()
{
  Serial.println("[TASK] Starting Task softwareReset");
  ThisThread::sleep_for(TIME_BETWEEN_RESETS_MS);
  Serial.println("[RST] *******Recurrent Reset*******");
  //NVIC_SystemReset();//works!
  system_reset();//works!
}

void task_blinkLed13()
{

 Serial.println("[TASK] Starting Task blinkLed13");
 while (true) 
    {
        Watchdog::get_instance().kick();
        digitalWrite(13, HIGH);
        ThisThread::sleep_for(500);  
        digitalWrite(13, LOW);
        ThisThread::sleep_for(500);
    }
}

void task_reconnectingWIFI()
{
  Serial.println("[TASK] Starting Task reconectingWIFI");
  while(true)
  {
        
          if (WiFi.status() == WL_CONNECTED)
          {
            //Serial.println("[WIFI] WiFi OK");
            wifiOK = 1;
            digitalWrite(ledG, HIGH);
            digitalWrite(ledR, LOW);
            //return;
          }
        
          if (WiFi.status() != WL_CONNECTED)
          {
            wifiOK = 0;
            mqttOK = 0;
            digitalWrite(ledG, LOW);
            digitalWrite(ledR, HIGH);
            // Start connection to WLAN router and print a status value
            Serial.println("[WIFI] Trying to connect to WLAN router");
            WiFi.disconnect();
            client.disconnect();
            ThisThread::sleep_for(1000);
        
            status = WiFi.begin(ssid, pass);
            // WL_IDLE_STATUS     = 0
            // WL_NO_SSID_AVAIL   = 1
            // WL_SCAN_COMPLETED  = 2
            // WL_CONNECTED       = 3
            // WL_CONNECT_FAILED  = 4
            // WL_CONNECTION_LOST = 5
            // WL_DISCONNECTED    = 6
            ThisThread::sleep_for(10000);
            Serial.print("[WIFI] Wifi status= ");
            Serial.println (status);
            if (status == WL_CONNECTED)
            {
                wifiOK = 1;
                Serial.println("[WIFI] Connection to WLAN router successful");
                printCurrentNet();
                printWifiData();
            }
          }

          CheckMQTT();
          ThisThread::sleep_for(10);     
  }
}

void CheckMQTT()
{
  if (client.connected() && wifiOK)
  {
    mqttOK = 1;
    client.loop();
    //Serial.println("MQTT OK");
  }

  if (client.connected() && !wifiOK)
  {
    mqttOK = 0;
  }

  if (!client.connected() && !wifiOK)
  {
    mqttOK = 0;
  }
  
  if (!client.connected() && wifiOK)
  {
    client.disconnect();
    Serial.print("[MQTT] Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "yourClientName";
    if (client.connect(clientId.c_str())) 
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("/RP2040/outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("/RP2040/inTopic");
    } else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in  seconds");
      client.disconnect();
    }
  }
}

//*************** mqtt functions ******************************
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();  
}

void publishState()
{
  char estat[256];
  memset(estat,0,256);//esborra estat
  unsigned long upTime = millis()/1000;
  
  doc["uT"] = upTime;
  
  serializeJson(doc, estat);
  if (mqttOK) client.publish("/RP2040/state", estat);
  Serial.print("[STAT] ");
  Serial.println(estat);
}

//************* other functions ***************************
void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("[WIFI] IP Address: ");
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("[WIFI] MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("[WIFI] SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("[WIFI] BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("[WIFI] signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("[WIFI] Encryption Type:");
  Serial.println(encryption, HEX);
  //Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}


//******************* Ticker ******************************
void flipp()
{    
    ticTac = !ticTac;
}
