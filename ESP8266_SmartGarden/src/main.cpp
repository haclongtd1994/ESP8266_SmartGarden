/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "haclongtd1994"
#define WLAN_PASS       "hunglapro1329"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "haclongtd1994"
#define AIO_KEY         "aio_VTME684H9f8mST4NzMpJudb9CfWp"
#define MQTT_CONN_KEEPALIVE 300

/************************* Global Time Setup *********************************/
const long utcOffsetInSeconds = 25200;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish Status_Stream = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/status-stream");
Adafruit_MQTT_Publish MultiTextReport = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/multitextreport");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton_man = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoffbutton_man");

Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Socket_1");
Adafruit_MQTT_Subscribe adjuststartvalue_h = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/adjuststart_value_h");
Adafruit_MQTT_Subscribe adjuststartvalue_m = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/adjuststart_value_m");
Adafruit_MQTT_Subscribe adjuststopvalue_h = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/adjuststop_value_h");
Adafruit_MQTT_Subscribe adjuststopvalue_m = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/adjuststop_value_m");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();
void Publish_Value_x(uint32_t x);
void Publish_Log();

/*************************** Variable ************************************/
volatile uint32_t Automate_Flag = 0;
volatile uint32_t Manual_Set = 0;
String AdjStartValueString_h = "9";
String AdjStartValueString_m = "30";
String AdjStopValueString_h = "9";
String AdjStopValueString_m = "40";
volatile uint32_t x=0;

char buffer [5000] = {0};
int cx;
int ind = 0;

/*************************** Setup Code ************************************/
void setup() {
  Serial.begin(115200);
  delay(20);
  Serial.println();

  Serial.println(F("Adafruit MQTT Smart Garden"));
  cx = snprintf ( buffer + ind, 5000, "Adafruit MQTT Smart Garden\n");

  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(0,HIGH);
  digitalWrite(2,HIGH);

  // Connect to WiFi access point.
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  ind += cx;
  cx = snprintf ( buffer + ind, 5000-ind, "Connecting to %s\n", WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ind += cx;
  cx = snprintf ( buffer + ind, 5000-ind, "WiFi connected\nIP address: %s\n", WLAN_SSID);

  // Start timeclient
  timeClient.begin();

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);
  mqtt.subscribe(&onoffbutton_man);
  mqtt.subscribe(&adjuststartvalue_h);
  mqtt.subscribe(&adjuststartvalue_m);
  mqtt.subscribe(&adjuststopvalue_h);
  mqtt.subscribe(&adjuststopvalue_m);
}

/*************************** Loop Code ************************************/
void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  static bool IsUpdated_b = false;
  static bool IsON_b = false;
  static bool firsttime = true;
  MQTT_connect();
  timeClient.update();

  if (firsttime){
    Publish_Log();
    firsttime = false;
  }

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Automation Mode(0:ON, 1:OFF): "));
      Serial.println((char *)onoffbutton.lastread);
      ind += cx;
      cx = snprintf ( buffer + ind, 5000-ind, "Automation Mode(0:ON, 1:OFF): %s\n", (char *)onoffbutton.lastread);

      if (atoi((char *)onoffbutton.lastread) == 0)
      {
        Automate_Flag = 1;
      }
      else
      {
        Automate_Flag = 0;
      }
    }
    if (subscription == &adjuststartvalue_h)
    {
      Serial.print(F("Hour Start: "));
      AdjStartValueString_h = String((char *)adjuststartvalue_h.lastread);
      Serial.println(AdjStartValueString_h);
      ind += cx;
      cx = snprintf ( buffer + ind, 5000-ind, "Hour Start: %u\n", *adjuststartvalue_h.lastread);
    }
    if (subscription == &adjuststartvalue_m)
    {
      Serial.print(F("Min Start: "));
      AdjStartValueString_m = String((char *)adjuststartvalue_m.lastread);
      Serial.println(AdjStartValueString_m);
      ind += cx;
      cx = snprintf ( buffer + ind, 5000-ind, "Min Start: %u\n", *adjuststartvalue_m.lastread);
    }
    if (subscription == &adjuststopvalue_h)
    {
      Serial.print(F("Hour Stop: "));
      AdjStopValueString_h = String((char *)adjuststopvalue_h.lastread);
      Serial.println(AdjStopValueString_h);
      ind += cx;
      cx = snprintf ( buffer + ind, 5000-ind, "Hour Stop: %u\n", *adjuststopvalue_h.lastread);
    }
    if (subscription == &adjuststopvalue_m)
    {
      Serial.print(F("Min Stop: "));
      AdjStopValueString_m = String((char *)adjuststopvalue_m.lastread);
      Serial.println(AdjStopValueString_m);
      ind += cx;
      cx = snprintf ( buffer + ind, 5000-ind, "Min Stop: %u\n", *adjuststopvalue_m.lastread);
    }

    if (subscription == &onoffbutton_man)
    {
      Serial.print(F("Manual value: "));
      Serial.println((char *)onoffbutton_man.lastread);
      if (atoi((char *)onoffbutton_man.lastread) == 0)
      {
        Manual_Set = 1;
      }
      else
      {
        Manual_Set = 0;
      }
    }

    // Set the value to true
    IsUpdated_b = true;
  }


  // Process auto and manual mode
  if (IsUpdated_b) {
    IsUpdated_b = false;

    if ( Automate_Flag)
    {
      // Mode: Auto
      Serial.println("Mode: Auto");
      Serial.print("Current Time: ");
      Serial.print(timeClient.getHours());
      Serial.print(":");
      Serial.print(timeClient.getMinutes());
      Serial.print(":");
      Serial.println(timeClient.getSeconds());
      ind += cx;
      cx = snprintf ( buffer + ind, 5000-ind, "Mode: Auto\nCurrent Time: %d:%d:%d\n", timeClient.getHours(), timeClient.getMinutes(),timeClient.getSeconds());
      Serial.print("Setting of Start Time: ");
      Serial.print((int)AdjStartValueString_h.toInt());
      Serial.print(":");
      Serial.println((int)AdjStartValueString_m.toInt());
      Serial.print("Setting of Stop Time: ");
      Serial.print((int)AdjStopValueString_h.toInt());
      Serial.print(":");
      Serial.println((int)AdjStopValueString_m.toInt());
      ind += cx;
      cx = snprintf ( buffer + ind, 5000-ind, "Setting of Start Time: %d:%d\nSetting of Stop Time: %d:%d\n", (int)AdjStartValueString_h.toInt(),
                    (int)AdjStartValueString_m.toInt(), (int)AdjStopValueString_h.toInt(), (int)AdjStopValueString_m.toInt());
    }
    else if (!Automate_Flag)
    {
      // Mode: Manual
      Serial.println("Mode: Manual");
      if (Manual_Set)
      {
        x = 1;
        Publish_Value_x(x);
        Serial.println("Motor & VAN: ON");
        ind += cx;
        cx = snprintf ( buffer + ind, 5000-ind, "Motor & VAN: ON");
        digitalWrite(0,LOW);
        delay(300);
        digitalWrite(2,LOW);
      }
      else if (!Manual_Set)
      {
        x = 0;
        Publish_Value_x(x);
        Serial.println("Motor & VAN: OFF");
        ind += cx;
        cx = snprintf ( buffer + ind, 5000-ind, "Motor & VAN: OFF");
        digitalWrite(0,HIGH);
        digitalWrite(2,HIGH);
      }
    }
    Publish_Log();
  }

  if (!IsON_b){
    if ((int)timeClient.getHours() >= (int)AdjStartValueString_h.toInt())
    {
      if (((int)timeClient.getMinutes() >= (int)AdjStartValueString_m.toInt()) &&
          ((int)timeClient.getMinutes() < (int)AdjStopValueString_m.toInt()))
      {
        x = 1;
        Publish_Value_x(x);
        Serial.println("Motor & VAN: ON");
        digitalWrite(0,LOW);
        delay(300);
        digitalWrite(2,LOW);
        IsON_b = true;
      }
    }
  }
  else {
    if ((int)timeClient.getHours() >= (int)AdjStopValueString_h.toInt())
      {
        if ((int)timeClient.getMinutes() >= (int)AdjStopValueString_m.toInt())
        {
          x = 0;
          Publish_Value_x(x);
          Serial.println("Motor & VAN: OFF");
          digitalWrite(0,HIGH);
          digitalWrite(2,HIGH);
          IsON_b = false;
        }
      }
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

}

void Publish_Value_x(uint32_t value)
{
  // Now we can publish stuff!
  Serial.print(F("\nSending status stream val "));
  Serial.print(value);
  Serial.print("...");
  if (! Status_Stream.publish(value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}

void Publish_Log(){
  Serial.print(F("\nSending log to multiline text box "));
  Serial.print("...");
  if (! MultiTextReport.publish(buffer)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  for (int i=0; i<ind; i++){
    buffer[i] = 0;
  }
  cx = 0;
  ind = 0;
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}