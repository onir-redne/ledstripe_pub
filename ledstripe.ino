/*
 * IoT ESP8266 Based Mood Lamp (RGB LED) Controller Program
 * https://circuits4you.com
 */
 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include "config.h"
#include "index.h"
#include "ledstripectl.h"

//SSID and Password of your WiFi router
const char* ssid = "boromir";
const char* password = "3pi5c0pu5";

//ESP8266WebServer server(80);

//LED Connections
const int BlueLED_L = 16; // D0
const int RedLED_L = 5;    // D1 
const int GreenLED_L = 4; // D2
const int BlueLED_R = 0; // D3
const int RedLED_R = 14;    // D5
const int GreenLED_R = 12; // D6
const int intLED = 2; // D4


LedStripeCtl led_stipe_L = LedStripeCtl(RedLED_L, GreenLED_L, BlueLED_L, PWM_DUTY_CYCLE);
LedStripeCtl led_stipe_R = LedStripeCtl(RedLED_R, GreenLED_R, BlueLED_R, PWM_DUTY_CYCLE);
Ticker timer_leds = Ticker();

void TickUpdateStipes(void)
{
  led_stipe_L.Update();
  led_stipe_R.Update();
}
//=======================================================================
//                    handles main page
//=======================================================================
/*void handleRoot() {
  Serial.println("Root Page Requested");
  server.send(200, "text/html", MAIN_page);
}*/

//=======================================================================
//                    Handle Set Color
//=======================================================================
/*void handleForm() {
  //Saperate Colors are sent through javascript
  String red = server.arg("r");
  String green = server.arg("g");
  String blue = server.arg("b");
  int r = red.toInt();
  int g = green.toInt();
  int b = blue.toInt();

  Serial.print("Red:");Serial.println(r);
  Serial.print("Green:");Serial.println(g);
  Serial.print("Blue:");Serial.println(b);
  
  //PWM Correction 8-bit to 10-bit
  r = r * 4; 
  g = g * 4;
  b = b * 4;

  //for ULN2003 or Common Cathode RGB Led not needed
//  r = 1024 - r;
//  g = 1024 - g;
//  b = 1024 - b;
  
  //ESP supports analogWrite All IOs are PWM
  analogWrite(RedLED_L,r);
  analogWrite(GreenLED_L,g);
  analogWrite(BlueLED_L,b);
  analogWrite(RedLED_R,r);
  analogWrite(GreenLED_R,g);
  analogWrite(BlueLED_R,b);

  //server.sendHeader("Location", "/");
  //server.send(302, "text/plain", "Updated-- Press Back Button");
 
  delay(250);  
}*/
//=======================================================================
//                    SETUP
//=======================================================================
void setup()
{
  Serial.begin(115200);   //Start serial connection  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("***** LEDSTRIPCTL *****");
  Serial.print("Connected to ");  
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  
  //server.on("/", handleRoot);  //Associate handler function to path
  //server.on("/setRGB",handleForm);
    
  //server.begin();                           //Start server
  //Serial.println("HTTP server started");

  Serial.println("setting up sample transitions...)");
  //led_stipe_L.AddTransition(  0,   0,   0,  /*->*/  255,   0,   0, 255);
  //led_stipe_L.AddTransition(255,   0,   0,  /*->*/    0, 255,   0, 255);
  //led_stipe_L.AddTransition(  0, 255,   0,  /*->*/    0,   0, 255, 255);
  //led_stipe_L.AddTransition(    0,   0,   0,  /*->*/    0,   0,   255, 0);

  led_stipe_L.SetColor(255, 255, 255);

  led_stipe_R.AddTransition(255, 127,   0,  /*->*/   127,   0, 127, 1000);
  led_stipe_R.AddTransition(127,   0, 127,  /*->*/     0, 127, 255, 1000);
  led_stipe_R.AddTransition(  0, 127, 255,  /*->*/   127, 255, 127, 1000);
  led_stipe_R.AddTransition(127, 255, 127,  /*->*/   255, 127,   0, 1000);

  // attach timer to call led update
  timer_leds.attach_ms(25, TickUpdateStipes);
}
//=======================================================================
//                    LOOP
//=======================================================================
void loop()
{
  //server.handleClient();
}
