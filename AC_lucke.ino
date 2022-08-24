#include "PubSubClient.h"
#include "FastLED.h"
#include "WiFi.h"
#include "ArduinoUniqueID.h"
#include "esp32fota.h"
#include "time.h"



#define LED_BUILTIN 5  // pro mini = 13 // NodeMcu-12 ESP8266 = 2 ESP32 = 2 esp32 sem dal na pin 25
#define STOP_TIPKA 33 // prej je bilo 26, zdaj sem dal na 14
#define CHARGING_PIN 26
boolean stop_tipka_status = 0;
#define LED_PIN 27

#define TIPKA_VKLOP 2
boolean vklop_tipka_status = 0;
boolean izvaja_se_vklop_na_tipko = 0;


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
const char* ssid = "implera_cs";
const char* password = "iMplera!";

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



boolean statusCharging = LOW;

uint8_t gBrightness_charging = 100;
uint8_t gBrightness_stop_button = 100;

uint8_t signalna = 0;

uint8_t animacija_EMButton = 1;

uint8_t p_offline = 0;
uint8_t led_trak_veja = 0;
uint8_t charging_active = 0;
uint8_t rezervacija = 0;
uint8_t outlet = 1;
uint8_t soc = 33;

#define GPIO23 23
#define GPIO5 5
#define GPIO18 18
#define GPIO19 19

#define NUM_STATUS_PREKAT 0
#define NUM_CHARGING_PREKAT 49
#define NUM_LEDS_ON_PREKAT 1

#define WS_LEDS_PIN 32

CRGB leds[NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT];

uint8_t demo_program = 0;
unsigned long lastTime_check_charging = 0;

CLEDController *controllers[1];

// neke barve, ki jih uporabljam ...
CRGB moja_barva_rdeca = CRGB(255, 0, 0);
CRGB moja_barva_zelena = CRGB(0, 255, 0);
CRGB moja_barva_modra = CRGB(0, 0, 255);
CRGB moja_barva_napolnjeno = CRGB(0, 255, 0);
CRGB moja_barva_prazno = CRGB(0, 0, 255);
CRGB moja_barva_crna = CRGB(0, 0, 0);
CRGB moja_barva_bela = CRGB(255, 255, 255);
CRGB moja_barva_orange = CRGB(255, 165, 0);
CRGB moja_barva_asdf1 = CRGB(100, 100, 100);

#define UPDATES_PER_SECOND 100
#define FRAMES_PER_SECOND 60

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


bool gReverseDirection = false;



void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;

    for( int i = 0; i < (NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT); i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;

}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;

    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
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

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STOP_TIPKA, INPUT_PULLDOWN);
  pinMode(TIPKA_VKLOP, INPUT_PULLDOWN);
  pinMode(LED_PIN, OUTPUT);
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
  

  controllers[0] = &FastLED.addLeds<NEOPIXEL, WS_LEDS_PIN>(leds, 0, NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT );
  FastLED.setBrightness(0);
  FastLED.show();
  off_all(0);
  on_all(10);
  off_all(10);
  
  if( stop_tipka_status == 0 ) {
    set_charging_led();
  }

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  esp32FOTA.checkURL = "http://lockit.pro/ota/ACLights/ACLights.json";

  FastLED.setBrightness( 100 );

  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;

  for(int kkk = 0; kkk < 70; kkk++){
//---------------------PALLETE--------------------------//
//    ChangePalettePeriodically();
//
//    static uint8_t startIndex = 0;
//    startIndex = startIndex + 1; /* motion speed */
//
//    FillLEDsFromPaletteColors( startIndex);
//
//    FastLED.show();
//    FastLED.delay(1000 / UPDATES_PER_SECOND);
//-----------------------------------------------------//

//---------------------NIGHT RIDER V2-------------------//

for(int z=0; z < 100; z++){
  uint16_t sinBeat = beatsin16(30, 0, (NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT) - 1, 0, 0);

  leds[sinBeat] = CRGB::Red;
  
  fadeToBlackBy(leds, (NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT), 10);

  EVERY_N_MILLISECONDS(10){
    Serial.println(sinBeat);
  }

  FastLED.show();
}
  

//--------------------NIGHT RIDER-----------------------//
//      nightRider(CRGB(255, 0, 0), CRGB(0, 0, 0), 3, 80);   //CRGB2 (255,255,155-205) == BELA barva
//------------------------------------------------------//

//-------------------FIRE---------------------------//    
//    Fire2012(); // run simulation frame
//
//    FastLED.show(); // display this frame
//    FastLED.delay(1000 / FRAMES_PER_SECOND);
//-------------------------------------------------//
  }

  
  
  Serial.println("END setup");

//  delay(500);
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
 
//  signal_led_on_off(); // loop vsako izvajanje nastavi lučko na drugačno stanje ... se pravi en loop se prižge , drugi loop se ugasne

    statusCharging = digitalRead(CHARGING_PIN);
  Serial.print("charging pin = ");
  Serial.println(statusCharging);
  if(statusCharging == HIGH){
    demo_program = 0;
  }else if(statusCharging == LOW){
    demo_program = 1;
  }

  if( demo_program == 0 ){
    p_offline = 0;
    led_trak_veja = 0;
    charging_active = 1;
    rezervacija = 0;
    outlet = 1;
    soc = 100;
  }
  else if( demo_program == 1 ){
    p_offline = 0;
    led_trak_veja = 0;
    charging_active = 1;
    rezervacija = 0;
    outlet = 1;
    soc = 1;
  }
//
//  else if ( demo_program == 2 ) {
//    p_offline = 0;
//    led_trak_veja = 0;
//    charging_active = 1;
//    rezervacija = 0;
//    outlet = 1;
//    soc = 77;
//  }
//  else if ( demo_program == 3 ) {
//    p_offline = 0;
//    led_trak_veja = 0;
//    charging_active = 0;
//    rezervacija = 1;
//    outlet = 1;
//    soc = 0;
//  }
//  else if ( demo_program == 4 ) {
//    p_offline = 1;
//    led_trak_veja = 0;
//    charging_active = 0;
//    rezervacija = 0;
//    outlet = 1;
//    soc = 0;
//  }
//  else {
//    p_offline = 0;
//    led_trak_veja = 0;
//    charging_active = 1;
//    rezervacija = 0;
//    outlet = 1;
//    soc = 66;
//  }

//  if ((millis() - lastTime_check_charging) > 10000) {
//    lastTime_check_charging = millis();
//    demo_program = demo_program + 1;
//    if(demo_program > 4){
//      demo_program = 0;
//    }
//  }


  
  stop_tipka_status = digitalRead(STOP_TIPKA);
  //Serial.println(stop_tipka_status);
  if( stop_tipka_status == HIGH ){
      if( animacija_EMButton != 1 ){
        FastLED.setBrightness(gBrightness_stop_button);// global brightness
        prizgiStatusneLucke( moja_barva_rdeca );
      }
  } else {
    FastLED.setBrightness(gBrightness_charging);// global brightness
  }

  vklop_tipka_status = digitalRead(TIPKA_VKLOP);
  Serial.print("vklop_tipka_status: ");
  Serial.println(vklop_tipka_status);
  if( vklop_tipka_status == HIGH ){
    
    if( izvaja_se_vklop_na_tipko == 0 ){
      
      izvaja_se_vklop_na_tipko = 1;

    }
  }
//
//  if( p_offline == 1 ){
//    for (int i = 0; i < NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT; ++i) {
//      leds[i] = moja_barva_rdeca;
//    }
//    FastLED.show();
//  }
//
//  else if( stop_tipka_status == 1  && p_offline == 0 ){
//    if( animacija_EMButton == 1 ){
//      animacija_em_button_1();
//    } 
//  }

//  else if( stop_tipka_status == 0  && p_offline == 0 && charging_active == 0  ){
//    if( rezervacija == 1 ){
//      //off_all(0);
//      prizgiStatusneLucke(moja_barva_modra);
//    }
//  }
//  else if( stop_tipka_status == 0  && p_offline == 0 && charging_active == 1   ){
//    prizgiStatusneLucke(moja_barva_orange); // moja_barva_orange  moja_barva_zelena
//  }
  
  // imaš vse podatke .. update lučke ..
//  Serial.println("Imaš vse podatke .. za .. update lučke .. :");
//  Serial.print("gBrightness_stop_button: ");
//  Serial.print(gBrightness_stop_button);
//  Serial.print("  |  gBrightness_charging: ");
//  Serial.print(gBrightness_charging);
//  Serial.print("  |  stop_tipka_status: ");
//  Serial.print(stop_tipka_status);
//  Serial.print("  |  p_offline: ");
//  Serial.println(p_offline);
//
//  Serial.print("           | outlet: ");
//  Serial.print(outlet);
//  Serial.print("                  | soc: ");
//  Serial.print(soc);
//  Serial.print("                | led_trak_veja: ");
//  Serial.println(led_trak_veja);
//  
//  Serial.print("charging_active: ");
//  Serial.print(charging_active);
//  Serial.print("               | animacija_EMButton: ");
//  Serial.print(animacija_EMButton);
//  
//  Serial.print("rezervacija: ");
//  Serial.println(rezervacija);

  FastLED.setBrightness(gBrightness_stop_button);
  FastLED.setBrightness(gBrightness_charging);

  if( stop_tipka_status == 0 && p_offline == 0) {
    set_charging_led();
  }

  delay(200);
//  Serial.println("loop END");
//  Serial.println("####################################################");
//  Serial.println(" ");
}

void signal_led_on_off() {
  // loop vsako izvajanje nastavi lučko na drugačno stanje ... se pravi en loop se prižge , drugi loop se ugasne
  if( signalna == 0 ){
    digitalWrite(LED_BUILTIN, HIGH);
    signalna = 1;
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
    signalna = 0;
  }
}

// switches off all LEDs
void off_all(long delayTime) {
  for (int i = 0; i < NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT ; ++i) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(delayTime);
}

void on_all(long delayTime) {
  if( stop_tipka_status == 1 ) {
 
  } else {
    for (int i = 0; i < NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT; ++i) {
      leds[i] = CRGB::Blue;
      FastLED.show();
      //Serial.println("TUKAJ2");
      delay(30);
    }
  }

  FastLED.show();
  delay(delayTime);
}


void set_charging_led() {
  Serial.println("set_charging_led START ");
  if( ( charging_active == 0 ) && rezervacija == 0 ){
    Serial.println("set ALL moja_barva_zelena");

    for (int i = 0; i < NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT; ++i) {
      leds[i] = moja_barva_crna;
      //Serial.println("set CHARGING moja_barva_crna");
    }
    
    for (int i = NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT; i < NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT; ++i) {
      leds[i] = moja_barva_modra;
      //Serial.println("set STATUS moja_barva_zelena");
    }
  }

  if(charging_active == 1 && soc > 0 ){
    Serial.println(" led_trak_veja == 0 && charging_active == 1 && soc > 0 ");
    // spremeni soc v stevilo prizganih ledic

    int prizganihPrekatovCharging = 0;
    prizganihPrekatovCharging = ( ( ( soc * 32 + 16 ) * NUM_CHARGING_PREKAT  ) / 100 ) / 32 ;
    
    Serial.print("prizganihPrekatovCharging = ");
    Serial.println(prizganihPrekatovCharging);
    
    int prekatPoVrsti = 0;
    for (int i = 0; i < NUM_CHARGING_PREKAT; i++) {
      if(prekatPoVrsti < prizganihPrekatovCharging){
        prekatPoVrsti++;
        //leds[i] = moja_barva_modra;
        prizgi_prekat(i, moja_barva_napolnjeno);
        //Serial.println("## moja_barva_modra ##");
        //delay(1000);
      } else {
        //leds[i] = moja_barva_crna;
        prizgi_prekat(i, moja_barva_crna);
        //Serial.println("## moja_barva_crna ##");
        delay(3);
      }
    }

    FastLED.show();

    prekatPoVrsti = 0;
    for (int i = 0; i < NUM_CHARGING_PREKAT; i++) {
      if(prekatPoVrsti < prizganihPrekatovCharging){
        prekatPoVrsti++;
        //Serial.println("## skip ##");
        //delay(1000);
      } else {
        prizgi_prekat(i, moja_barva_prazno);
        //Serial.println("## moja_barva_orange ##");
        //delay(1000);

        if( NUM_CHARGING_PREKAT  <  20 ){
          delay(30);
        } else if ( NUM_CHARGING_PREKAT > 19 &&  NUM_CHARGING_PREKAT  <  50 ) {
          delay(30);
        } else if ( NUM_CHARGING_PREKAT > 49 &&  NUM_CHARGING_PREKAT  <  80 ) {
          delay(80);
        } else {
          delay(40);
        }

        FastLED.show();
      }
    }

    FastLED.show();
  }

  FastLED.show();
}


void prizgi_prekat(int prekat, CRGB barva) {

  for (int i = prekat * NUM_LEDS_ON_PREKAT; i < (prekat + 1) * NUM_LEDS_ON_PREKAT; i++) {
    //Serial.println("## sadfasdf ##");
    leds[i] = barva;
  }
  FastLED.show();
  
}

void prizgiStatusneLucke( CRGB barva ){
  for (int i = NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT; i < NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT; i++) {
    //Serial.println("## sadfasdf ##");
    leds[i] = barva;
  }
  FastLED.show();      
}


void animacija_em_button_1() {
  Serial.println("animacija_em_button_1 START ");
  off_all(0);
  int wiat_millis = 60;
//  if( stop_tipka_status == 1 ) {
      prizgiStatusneLucke( moja_barva_rdeca );

      for (int j = 0; j < 100; j = j + 5) {
        FastLED.setBrightness(j);
        FastLED.show();
        delay(wiat_millis);
      }
      for (int k = 100; k > 0; k = k - 5) {
        FastLED.setBrightness(k);
        FastLED.show();
        delay(wiat_millis);
      }

      FastLED.setBrightness(0);

      prizgiStatusneLucke( moja_barva_zelena );

      for (int j = 0; j < 60; j = j + 5) {
        FastLED.setBrightness(j);
        FastLED.show();
        delay(wiat_millis);
      }
      for (int k = 60; k > 10; k = k - 5) {
        FastLED.setBrightness(k);
        FastLED.show();
        delay(wiat_millis);
      }
    
//  }
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



// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120


void Fire2012()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / (NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT)) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= (NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT) - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < (NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT); j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = ((NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT)-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
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
  for (int i=0; i<(NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT); i++) {  //FORWARD
    leds[i] = c;
    if (i-len >= 0) {
      leds[i-len] = c2;
    }
    FastLED.show();
    delay(delayTime);
  }
  for (int i=0; i<(NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT); i++) {  //BACKWARD
    leds[(NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT)-i] = c;
    if (((NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT)-i) <= (NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT)-len) {
      leds[((NUM_CHARGING_PREKAT * NUM_LEDS_ON_PREKAT + NUM_STATUS_PREKAT * NUM_LEDS_ON_PREKAT)-i) + len - 1] = c2;
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
