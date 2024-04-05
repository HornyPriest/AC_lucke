#include "PubSubClient.h"
#include "FastLED.h"
#include "WiFi.h"
#include "ArduinoUniqueID.h"
#include "esp32fota.h"
#include "time.h"




#define CHARGING_PIN 26
#define WS_LEDS_PIN 32

#define LEDS_COUNT 109


boolean statusCharging = LOW;
uint8_t gBrightness_charging = 100;

CRGB leds[LEDS_COUNT];

CLEDController *controllers[1];


// esp32fota esp32fota("<Type of Firme for this device>", <this version>);
esp32FOTA esp32FOTA("ACLights", 1);

int FW_version = 1;

// Second Core Settings//
TaskHandle_t Task1;
//////////////////////////

String deviceName = "ACLights-";
const char * chardeviceName;
char charid[23];
String idTopic;
String epochtimeTopic;
String prefix = "ACLights/";
String fullTopic;
String dynamicTopic;


String StartupTopic = "/startup/";
String ConfirmationTopic = "/confirmation/";
String FWversionTopic = "/FW_verison";
String RecconectTopic = "/Reconnected";
String TempTopic;

const char * topica;
const char * ConfirmationStrChar;

const char * charTest;
String sub_TestTopic;
const char * charUpdate;
String sub_UpdateTopic;


boolean STARTUP = HIGH;
boolean UpdateStart;
boolean Test;
int TempTest;
boolean ConfirmationSend;
String ConfirmationString;

const char * TempValueChar;

// Replace the next variables with your SSID/Password combination
const char* ssid = "Implera";
const char* password = "AdminSettings";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "mqshare.napolni.me";
String temp_message;

// NTP server settings
const char* ntpServer = "pool.ntp.org";
//unsigned long epochTime;
long unsigned int epochTime;
const int  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

WiFiClient espClient;
PubSubClient client(espClient);
int lastMsg = 0;
char msg[50];
int value = 0;

unsigned long startTime = millis();

const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};


unsigned long getTime()
{
  time_t now;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
//    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}


void setup() {
  delay(1000);
  Serial.begin(115200);

  pinMode(WS_LEDS_PIN, OUTPUT);
  pinMode(CHARGING_PIN, INPUT_PULLUP);
  


  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    100000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */

  uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(charid, 23, "%04X%08X", chip, (uint32_t)chipid);
  Serial.println(charid);
  delay(200);
  idTopic = charid;
  Serial.println(idTopic);
  deviceName += idTopic;
  chardeviceName = deviceName.c_str();

  sub_UpdateTopic += prefix;
  sub_UpdateTopic += "set_update";
  charUpdate = sub_UpdateTopic.c_str();

  sub_TestTopic += prefix;
  sub_TestTopic += "Test";
  charTest = sub_TestTopic.c_str();
  

  controllers[0] = &FastLED.addLeds<NEOPIXEL, WS_LEDS_PIN>(leds, 0, LEDS_COUNT);
  FastLED.setBrightness(0);
  FastLED.show();

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);



  FastLED.setBrightness( 100 );

  for(int kkk = 0; kkk < 70; kkk++){
    //---------------------NIGHT RIDER V2-------------------//
    
    for(int z=0; z < 100; z++){
      uint16_t sinBeat = beatsin16(30, 0, (LEDS_COUNT) - 1, 0, 0);
    
      leds[sinBeat] = CRGB::Red;
      
      fadeToBlackBy(leds, (LEDS_COUNT), 10);
    
      EVERY_N_MILLISECONDS(10){
        Serial.println(sinBeat);
      }
    
      FastLED.show();
    }
  }

  FastLED.setBrightness(gBrightness_charging);// global brightness
  
  Serial.println("END setup");
}

void setup_wifi() {
  WiFi.mode(WIFI_OFF);
  delay(3000);
  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_MODE_STA);
  delay(3000);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int ij;
  while (WiFi.status() != WL_CONNECTED && ij<25) {
    delay(500);
    ij = ij + 1;
    Serial.print(".");
  }
  delay(3000);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
  
  if (String(topic) == sub_UpdateTopic) {
    Serial.print("New update JSON link: ");
    Serial.println(messageTemp);
    temp_message = messageTemp;
    UpdateStart = HIGH;
  }
  
  if (String(topic) == sub_TestTopic) {
    Serial.println("Test received ");
    TempTest = messageTemp.toInt();
    ConfirmationString = "Test";
    ConfirmationString += " = ";
    ConfirmationString += TempTest;
    Test = HIGH;
    SendConfirmation();
  }
}

void reconnect() {
  // Loop until we're reconnected
  int i = 0;
  while (!client.connected() && i<10) {
    Serial.print("Attempting MQTT connection...");
    i = i + 1;
    // Attempt to connect
    if(i==10){
      setup_wifi();
    }
    if (client.connect(chardeviceName, "sharelock" , "biciklekomplikovanije")) {
      Serial.println("connected");
      selectTopic();
      fullTopic = dynamicTopic;
      fullTopic += RecconectTopic;
      topica = fullTopic.c_str();
      client.publish(topica, "1");
      // Subscribe
      client.subscribe(charUpdate);
      client.subscribe(charTest);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
// Wait 1 seconds before retrying
      delay(1000);
    }
  }
}




void selectTopic(){
    topica = "";
    dynamicTopic = "";
    epochtimeTopic = getTime();
    while(epochtimeTopic == 0){
      delay(100);
      epochtimeTopic = getTime();
      Serial.println(epochtimeTopic);
    }
    dynamicTopic += prefix;
    dynamicTopic += idTopic;
    dynamicTopic += "/";
    dynamicTopic += epochtimeTopic;
}


void loop() {

  if(STARTUP == HIGH && client.connected()){
    delay(1000);
    SendStartup();
    STARTUP = LOW;
  }

  if(UpdateStart == HIGH){
    if((WiFi.status() == WL_CONNECTED)) {
      boolean updatedNeeded = esp32FOTA.execHTTPcheck();
      if(updatedNeeded == HIGH){
        Serial.print("najden update");
        esp32FOTA.execOTA();
      }else{
        UpdateStart = LOW;
      }
    }
  }


  
  Serial.println("loop START");
 

  statusCharging = digitalRead(CHARGING_PIN);
  Serial.print("charging pin = ");
  Serial.println(statusCharging);
  if(statusCharging == HIGH){
    FastLED.setBrightness(255); // Max brightnes
    fill_solid(leds, LEDS_COUNT, CRGB::Green); // All LEDs green
  }else if(statusCharging == LOW){
    fill_solid(leds, LEDS_COUNT, CRGB::Blue); // All LEDs blue
    breathingEffect();
  }
  FastLED.show();
  delay(4);
}

void breathingEffect() {
  static uint8_t breathe = 25;
  static int8_t direction = 1;
  breathe += direction;
  if (breathe == 25 || breathe == 255) {
    direction = -direction;
  }
  FastLED.setBrightness(breathe);
}


// switches off all LEDs
void off_all() {
  
}

void set_idle_led() {
  
}


void set_charging_led() {
  
}


void SendStartup(){
  String TempValue;
  
  delay(1000);
  Serial.println("Pošiljam Startup");
  selectTopic();
  delay(100);
  selectTopic();
  fullTopic = dynamicTopic;
  fullTopic += StartupTopic;
  topica = fullTopic.c_str();
  client.publish(topica, "123");
  fullTopic = dynamicTopic;
  fullTopic += FWversionTopic;
  topica = fullTopic.c_str();
  TempValue = "";
  TempValue += FW_version;
  TempValueChar = TempValue.c_str();
  client.publish(topica, TempValueChar);
}


void SendConfirmation(){
  Serial.println("Pošiljam Confirmation");
  selectTopic();
  fullTopic = dynamicTopic;
  fullTopic += ConfirmationTopic;
  topica = fullTopic.c_str();
  ConfirmationStrChar = ConfirmationString.c_str();
  client.publish(topica, ConfirmationStrChar);
}

/**
 * A nightrider effect
 * CRGB c
 *  color for the main group of LEDs
 *  default = CRGB(255, 0, 0)
 * CRGB c2
 *  color for the background
 *  default = CRGB(0, 0, 0);
 * int len
 *  length of the main group of LEDs
 *  default = 3
 * int delayTime
 *  time to wait before moving the main group of LEDs
 *  default = 50 (ms)
 */
void nightRider(CRGB c, CRGB c2, int len, int delayTime) {
  int i=0;
  for (int i=0; i<(LEDS_COUNT); i++) {  //FORWARD
    leds[i] = c;
    if (i-len >= 0) {
      leds[i-len] = c2;
    }
    FastLED.show();
    delay(delayTime);
  }
  for (int i=0; i<(LEDS_COUNT); i++) {  //BACKWARD
    leds[(LEDS_COUNT)-i] = c;
    if (((LEDS_COUNT)-i) <= (LEDS_COUNT)-len) {
      leds[((LEDS_COUNT)-i) + len - 1] = c2;
    }
    FastLED.show();
    delay(delayTime);
  }
}



void Task1code( void * pvParameters ){
  setup_wifi();
  delay(2000);
  client.setServer(mqtt_server, 31883);
  client.setCallback(callback);

  
  if(WiFi.status() == WL_CONNECTED) {
   /*   ArduinoOTA.setHostname(chardeviceName);
      setupOTA(chardeviceName, ssid, password);*/
  }

  for(;;){
    if (!client.connected()) {
      if (!WiFi.status() == WL_CONNECTED){
        setup_wifi();
      }
      reconnect();
    }
  client.loop();
  vTaskDelay(1000);
  } 
}
