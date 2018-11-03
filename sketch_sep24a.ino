// Wifi Stuff
#include <ESP8266HTTPClient.h>
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

// delays in milliseconds
#define ATTEMPT_REACHING_SERVER_PERIOD 5000
#define SENSOR_READ_PERIOD 60000
// delays in seconds
#define CONFIG_TIMEOUT 180

#define HOST "api.hackair.eu"
#define PORT 80

#define SERIAL_SPEED 9600

WiFiManager wifiManager;
boolean connected = false;

LiquidCrystal_I2C lcd(0x3F, WIDTH, HEIGHT);
//LiquidCrystal_I2C lcd(0x27, WIDTH, HEIGHT);

void init_display(int width, int height){  
  //lcd.begin(width, height);
  lcd.init();
  lcd.setCursor(0, 0);
  lcd.print("Booting...");
  
  // Turn on the backlight.
  lcd.backlight();
  // lcd.noBacklight(); // to turn off
}

void setupSerial(){
  // Initialize serial port
  Serial.begin(SERIAL_SPEED);
  while (!Serial) continue;
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
  lcd.print(myWiFiManager->getConfigPortalSSID());
}

void init_wifi(){
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  // set a timeout, so that the config portal won't be available if we forget to set it up (i.e. we prevent others to maliciously set it when we leave it open)
  wifiManager.setConfigPortalTimeout(CONFIG_TIMEOUT);

  // set callback that gets called once the user inserts parameters and the connection is successfully established
  // i.e. we have to save the parameters somewhere
  //wifiManager.setSaveConfigCallback(saveConfigCallback);

}

boolean attempt_connect(){
  boolean result = false;
  
  // if the inserted credentials were not correct
  if (!wifiManager.autoConnect()) {
    
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Connection");
    lcd.setCursor(4, 1);
    lcd.print("failed!");
  }

  // if we forget to access the configuration within CONFIG_TIMEOUT seconds
  // this will anyways unlock the autoConnect which will continue here

  // let's then check if we successfully connected
  if(WiFi.status() == WL_CONNECTED){
    result = true;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connected as");
    lcd.setCursor(3, 1);
    lcd.print(WiFi.localIP());

    // set LED on
    digitalWrite(BUILTIN_LED, LOW);
  }

  return result;
}

SDS011 my_sds;

void init_dust_sensor(int rx, int tx){
  my_sds.begin(rx, tx);
  //Serial.begin(9600);
}


float pm10, pm25;
int error;

void read_dust_sensor(){
  error = my_sds.read(&pm25, &pm10);
  if (!error) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("P2.5: " + String(pm25)+" uG/m3");
    lcd.setCursor(0, 1);
    lcd.print("P10: " + String(pm10)+" ug/m3");
  }
  delay(100);
}

// to quantify the size of the JsonBuffer
// https://arduinojson.org/v5/assistant/
// StaticJsonBuffer uses the stack
//StaticJsonBuffer<200> jsonBuffer;
// DynamicJsonBuffer uses the heap
//DynamicJsonBuffer  jsonBuffer(200);

void send_measurements(){
  // if we have correctly sampled measurements and we were connected
  if(!error){
    if(connected){
      /*
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      JsonObject& reading = root.createNestedObject("reading");

      reading["PM2.5_AirPollutantValue"] = String(pm25);
      reading["PM10_AirPollutantValue"] = String(pm10);
      
      String message;
      root.printTo(message);
      */
      String message = "{\"reading\": {\"PM2.5_AirPollutantValue\":\""+String(pm25)+"\",\"PM10_AirPollutantValue\":\""+String(pm10)+"\"}}";
      
      String server = "https://api.hackair.eu/sensors/push/measurements";
      String auth_key = "3007997345d6b622a56153da20103d663e50f8b75d41";
      
      Serial.println("Sending "+message+" to "+server);
      
      // send a POST to https://api.hackair.eu/sensors/push/measurements
      HTTPClient http;
      http.begin(server);
      // set headers
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Accept", "application/json");
      http.addHeader("Authorization", auth_key);
           
      //JsonObject& root = jsonBuffer.parseObject(json);
      int httpCode = http.POST(message);
  
      // get the response
      String response = http.getString();

      if(httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }else {
        Serial.printf("Error %d on POST: %s\n", httpCode, http.errorToString(httpCode).c_str());
      }
   
      http.end();
   }else{
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ERROR:");
      lcd.setCursor(0, 1);
      lcd.print("During data uplink");
   }
  }  
}



boolean check_connection_to_server(String host, int port){
  boolean result = false;
  WiFiClient client;
  if(!client.connect(host, port)){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error:");
    lcd.setCursor(0, 1);
    lcd.print("Server unreachable");
    client.stop();
    delay(ATTEMPT_REACHING_SERVER_PERIOD);
  }else{
    result = true;
    Serial.println(String(host)+":"+String(port)+" is reachable");
  }
  return result;
}



void setup() {
  setupSerial();
  
  // to reset the previously stored password for debugging reasons
  //WiFi.disconnect();

  // init the display
  init_display(WIDTH, HEIGHT);

  // attempt connecting to the wifi network
  init_wifi();
  // stay stuck as long as connection is not possible
  connected = attempt_connect();
  if(connected){
    connected = check_connection_to_server(HOST, PORT);
  }

  // we managed to connect, we can start the dust sensor
  //init_dust_sensor(D6, D7);
}

void loop() {
   //read_dust_sensor();
   // bypass sensor for debugging the net interface
   pm25 = 6.6;
   pm10 = 13.5;
   error = false;
   send_measurements();
   // wait for the specified time
   delay(SENSOR_READ_PERIOD);
}
