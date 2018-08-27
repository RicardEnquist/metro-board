#include <ESP8266WiFi.h> //Using library ESP8266WiFi v1.0: https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <ArduinoJson.h> //Using library ArduinoJson v5.13.0: https://github.com/bblanchon/ArduinoJson
#include <Adafruit_SSD1306.h> //Using library Adafruit SSD1306 v1.1.2: https://github.com/adafruit/Adafruit_SSD1306
#include <Adafruit_GFX.h> //Using library Adafruit GFX v1.2.3: https://github.com/adafruit/Adafruit-GFX-Library

  //--- Settings ---//
const char* ssid = "Agnard"; //Wifi name
const char* password = ""; //Wifi password
int updateInterval = 30; //Seconds between API-updates


  //--- Declare API-variables ---//
#define API_KEY "9f3c9bf4d5df4b7899d3367145f4e5ed"
#define API_URL "http://api.sl.se/api2/realtimedeparturesv4.json?key=9f3c9bf4d5df4b7899d3367145f4e5ed&siteid=9304&timewindow=5"
 

  //--- Declare Json-variables ---//
static char respBuffer[16384];


  //--- Declare OLED-variables ---//
#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);


  //--- Declare other variables ---//
unsigned long lastData = 0;
int ageOfData = 999;
String TimeToTcentral = " ";
String TimeFromTcentral = " ";



void setup() {
  Serial.begin(115200);

  //--- Configure display ---//
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(500);
  display.clearDisplay();
  display.display();
  delay(500);

  //--- Connect to WiFi ---//
  WiFi.disconnect();
  Serial.println("Begin");
  
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while(WiFi.status() != WL_CONNECTED)
  {
     delay(500);
     Serial.println(".");
  }
  Serial.println();

  Serial.println("Connected, IP address: ");
  Serial.println(WiFi.localIP());

}


void loop() {
   
   //--- Check for updating interval ---//
if(ageOfData >= updateInterval){
  
   //--- Establish connection to API ---//
  WiFiClient client;

  if(client.connect("api.sl.se", 80)){   
    client.println("GET /api2/realtimedeparturesv4.json?key=9f3c9bf4d5df4b7899d3367145f4e5ed&siteid=9304&timewindow=30 HTTP/1.1");
    client.println("Host: api.sl.se");
    client.println("Connection: close");
    client.println();
  }
  else{
    Serial.println("Connectiond to api.sl.se failed.");
  }
    
  client.flush();
  delay(10);


  //--- Read data from URL, omit header ---//
  
  bool endOfHeader = false;
  
  uint16_t index = 0;
  while(client.connected()){
    char c = client.read();
    if(c == '{' && endOfHeader == false){
      endOfHeader = true;
    }
    if(client.available() && endOfHeader == true){
      respBuffer[index++] = c;
      delay(1);
      }
  }
  respBuffer[index++] = '}';

  client.stop();


  //--- Parse json data ---//

  char * json = strchr(respBuffer, '{');

  DynamicJsonBuffer jBuffer;
  JsonObject& root = jBuffer.parseObject(json);
  JsonObject& ResponseData = root["ResponseData"];
  
  const char* ResponseData_LatestUpdate = ResponseData["LatestUpdate"];
  Serial.println(ResponseData_LatestUpdate);

  TimeToTcentral = " ";
  TimeFromTcentral = " ";

   for (int i=0; i <= 5; i++){
      JsonObject& ResponseData_Metros = ResponseData["Metros"][i];
      const char* ResponseData_Metros_Destination = ResponseData_Metros["Destination"]; // "Akalla"
      const char* ResponseData_Metros_DisplayTime = ResponseData_Metros["DisplayTime"]; // "3 min"
      String DisplayTime = ResponseData_Metros["DisplayTime"];
      
      if (String(ResponseData_Metros_Destination) == "Kungsträdgården"){
          TimeToTcentral = TimeToTcentral + ' ' + DisplayTime;
      }
      else if (String(ResponseData_Metros_Destination) == "Akalla"){
          TimeFromTcentral = TimeFromTcentral + ' ' + DisplayTime;
      }
   }
   
  Serial.println(TimeToTcentral);
  Serial.println(TimeFromTcentral);
  Serial.println("");

  lastData = millis();
}

  ageOfData = ((millis() - lastData)/1000);
  
  
  //--- Write data to display ---//

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Kungstradgarden");
  display.println(TimeToTcentral);
  display.setCursor(0,32);
  display.println("Akalla");
  display.println(TimeFromTcentral);
  display.display();
  delay(10);
}