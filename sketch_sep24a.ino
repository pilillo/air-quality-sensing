// Wifi Stuff
#include <ESP8266WiFi.h>
#include <DNSServer.h>     
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// dust sensor
#include <SDS011.h>

// LCD Stuff
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 

// display size
#define WIDTH 16
#define HEIGHT 2
//#define WIDTH 20
//#define HEIGHT 4

WiFiManager wifiManager;

LiquidCrystal_I2C lcd(0x3F, 16, 2);
//LiquidCrystal_I2C lcd(0x27, 20, 4);

void init_display(int width, int height){
  //lcd.begin(16,2);
  //lcd.begin(20,4);
  
  //lcd.begin(width, height);
  lcd.init();
  lcd.setCursor(0, 0);
  lcd.print("Booting...");
  
  // Turn on the backlight.
  lcd.backlight();
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  //Serial.println("Entered config mode");
  //Serial.println(WiFi.softAPIP());
  lcd.setCursor(0, 0);
  // start soft access point
  WiFi.softAPIP();
  lcd.print("Please connect:");

  lcd.setCursor(0, 1);
  //if you used auto generated SSID, print it
  //Serial.println(myWiFiManager->getConfigPortalSSID());
  lcd.print(myWiFiManager->getConfigPortalSSID());
}

void init_wifi(){
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
}

void attempt_connect(){
  
  if (!wifiManager.autoConnect()) {
    //Serial.println("failed to connect and hit timeout");
    lcd.setCursor(3, 0);
    lcd.print("Connection");
    lcd.setCursor(4, 1);
    lcd.print("failed!");
    
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }else{
    lcd.setCursor(2, 0);
    lcd.print("Successfully");
    lcd.setCursor(3, 1);
    lcd.print("connected!");
     //keep LED on
    digitalWrite(BUILTIN_LED, LOW);
  }
}

SDS011 my_sds;

void init_dust_sensor(int rx, int tx){
  my_sds.begin(rx, tx);
  //Serial.begin(9600);
}

void read_dust_sensor(){
  float pm10, pm25;
  int error;

  error = my_sds.read(&pm25, &pm10);
  if (!error) {
    lcd.setCursor(0, 0);
    lcd.print("P2.5: " + String(pm25));
    lcd.setCursor(0, 1);
    lcd.print("P10:  " + String(pm10));
  }
  delay(100);
}

void setup() {
  // to reset the previously stored password for debugging reasons
  //WiFi.disconnect();
  
  init_display(WIDTH, HEIGHT);
  //init_wifi();
  //attempt_connect();
  init_dust_sensor(D6, D7);
  
}

void loop() {
  /*
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("Sending data");
   delay(1000);
   lcd.setCursor(13, 0);
   lcd.print(".");
   delay(1000);
   lcd.setCursor(14, 0);
   lcd.print(".");
   delay(1000);
   lcd.setCursor(15, 0);
   lcd.print(".");
   delay(1000);
   */

   read_dust_sensor();
   delay(8000);
}
