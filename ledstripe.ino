/*
 * IoT ESP8266 Based Mood Lamp (RGB LED) Controller Program
 * https://circuits4you.com
 */
 
#include <inttypes.h>
#include  <cstring>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//#include <EEPROM.h>
#include <Ticker.h>
#include "config.h"
#include "LittleFS.h"
#include "ledstripestate.h"
#include "ledstripectl.h"

////////////////////////////////////////
//SSID and Password of your WiFi router
const char* ssid = "boromir";
const char* password = "3pi5c0pu5";

////////////////////////////////////////
// Web server
const char content_html[] = "text/html; charset=UTF-8";
const char content_css[]  = "text/css; charset=UTF-8";
const char content_js[]  = "text/javascript; charset=UTF-8";
const char content_plain[] = "text/plain";
const char content_ico[] = "image/vnd.microsoft.icon";
ESP8266WebServer server(80);

////////////////////////////////////////
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
LedStripeCtl * led_stripes[] = {&led_stipe_L, &led_stipe_R};

Ticker timer_leds = Ticker();

////////////////////////////////////////
// global led updater
void TickUpdateStipes(void)
{
  led_stipe_L.Update();
  led_stipe_R.Update();
}

////////////////////////////////////////
// Web server handlers
// root
void WebRootHandler(void)
{
  Serial.printf("HTTP: %s", server.uri().c_str()); 
  if(LittleFS.exists("www" + server.uri()))
  {
    fs::File file = LittleFS.open("www" + server.uri(), "r");
    if(file.isFile())
    { 
      const char * p_content = nullptr;
      if(strstr(file.fullName(), ".htm") || strstr(file.name(), ".html"))
        p_content = content_html;
      else if(strstr(file.fullName(), ".js") || strstr(file.fullName(), ".map"))
        p_content = content_js;
      else if(strstr(file.fullName(), ".css"))
        p_content = content_css;
      else if(strstr(file.fullName(), ".ico"))
        p_content = content_ico;
      else
        p_content = content_plain;  
      
      
      if (server.streamFile(file, p_content) != file.size()) 
        Serial.println("Sent less data than expected!");

      file.close();
      Serial.printf(" - 200 [%s]\r\n", p_content);
    }
    else
    {
      // 500
      server.send(500, content_plain, "Internal error");
      Serial.println(" - 500");
    }
  }
  else
  {
    // 404
    server.send(404, content_plain, "File not found");
    Serial.println(" - 404");
  }
}

// ajax
void WebAjaxSetColorHandler(void)
{
  Serial.printf("AJAX: %s", server.uri().c_str());
  int r = atoi(server.arg("r").c_str());
  int g = atoi(server.arg("g").c_str());
  int b = atoi(server.arg("b").c_str());
  int l = atoi(server.arg("l").c_str());

  if(l >= 0 && l <= sizeof(led_stripes) -1)
  {
    led_stripes[l]->SetColor(r, g, b);
    server.send(200, content_plain);
    Serial.println(" - 200");
    return;
  }

  server.send(500, content_plain, "Internal error");
  Serial.println(" - 500");
}

void WebAjaxGetStripesStateHandler(void)
{
  char json_buffer[sizeof(led_stripes) * JSON_STRIPE_STATE_MAX] = {0};
  int8_t i = 0;
  uint16_t offset = 0;
  char state = 'c';

  json_buffer[0] = '[';
  offset++;
  for(LedStripeCtl * sctl : led_stripes) 
  {
    LedStripeState &cc = sctl->GetColorState();
    uint8_t r = cc.GetColor_R();
    uint8_t g = cc.GetColor_G();
    uint8_t b = cc.GetColor_B();
    if(sctl->ActiveTransitions())
    offset += sprintf(json_buffer + offset, "{\"l\":%u,\"r\":%u,\"g\":%u,\"b\":%u,\"s\":\"%c\"},", i, r, g, b, state);
  }
  json_buffer[offset-1] = ']';
  offset++;

  server.send(200, content_plain, json_buffer);
  Serial.println(" - 200");
  return;
}

void WebAjaxSavedColorsAdd(void)
{
  Serial.printf("AJAX: %s", server.uri().c_str());
  int r = atoi(server.arg("r").c_str());
  int g = atoi(server.arg("g").c_str());
  int b = atoi(server.arg("b").c_str());

  server.send(500, content_plain, "Internal error");
  Serial.println(" - 500");
}

//////////////////////////////////////
// Setup
void setup()
{
  Serial.begin(115200);   //Start serial connection  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
  
  // Wait for connection
  Serial.printf("***** Waiting for WiFi ssid [%s]: ", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  // init filesystem and do basic checks
  // Get all information of your LittleFS
  Serial.print("***** Inizializing FS: ");

  if (LittleFS.begin())
    Serial.println("done.");
  else
    Serial.println("fail.");

  FSInfo fs_info;
  LittleFS.info(fs_info);
  Serial.println("File sistem info.");
  Serial.print("Total space:      ");
  Serial.print(fs_info.totalBytes);
  Serial.println("byte");
  Serial.print("Total space used: ");
  Serial.print(fs_info.usedBytes);
  Serial.println("byte");
  Serial.print("Block size:       ");
  Serial.print(fs_info.blockSize);
  Serial.println("byte");
  Serial.print("Page size:        ");
  Serial.print(fs_info.pageSize);
  Serial.println("byte");
  Serial.print("Max open files:   ");
  Serial.println(fs_info.maxOpenFiles);
  Serial.print("Max path lenght:  ");
  Serial.println(fs_info.maxPathLength);
  Serial.println();

  //If connection successful show IP address in serial monitor
  Serial.print("***** Loading ledstripes configuration: ");
  Serial.println("TODO");
 
  Serial.print("***** Settingup web server: ");
  server.on("/ajax/setcolor", WebAjaxSetColorHandler);    // set static color
  server.on("/ajax/getstripesstate",  WebAjaxGetStripesStateHandler);    // Get current static colors for all stripes
  server.on("/ajax/savedcolors_add", WebAjaxSavedColorsAdd);    // add colors to favotites (do not save to flash)
  //server.on("/ajax/getconfig", WebAjaxSetColorHandler);   // return configuration
  //server.on("/ajax/saveconfig", WebAjaxSetColorHandler);  // save configuration (try to cache it to limit flash writes)
  //server.on("/ajax/reset", WebAjaxSetColorHandler);       // reboot device
  //server.on("/ajax/poweroff", WebAjaxSetColorHandler);    // turn off
  //server.on("/ajax/poweron", WebAjaxSetColorHandler);     // turn on
  //server.on("/ajax/selecttranset", WebAjaxSetColorHandler); // set transition-set 
  //server.on("/ajax/setransition", WebAjaxSetColorHandler);  // set single transition
  server.onNotFound(WebRootHandler);  // handle all uri's but ajax
  server.begin();
  Serial.println("done");

  Serial.println("setting up sample transitions...)");
  // led_stipe_L.AddTransition(  0,   0,   0,  /*->*/  255,   0,   0, 8000);
  // led_stipe_L.AddTransition(255,   0,   0,  /*->*/    0, 255,   0, 500);
  // led_stipe_L.AddTransition(  0, 255,   0,  /*->*/    0,   0, 255, 500);
  // led_stipe_L.AddTransition(    0,   0, 255,  /*->*/ 255,   0,   255, 500);
  // led_stipe_L.AddTransition(  255,   0, 255,  /*->*/    127,   127,   0, 500);
  // led_stipe_L.AddTransition(    127,   127,   0,  /*->*/    0,   0,   0, 500);

  //led_stipe_L.SetColor(255, 255, 255);

  // led_stipe_R.AddTransition(255, 127,   0,  /*->*/   127,   0, 127, 1000);
  // led_stipe_R.AddTransition(127,   0, 127,  /*->*/     0, 127, 255, 1000);
  // led_stipe_R.AddTransition(  0, 127, 255,  /*->*/   127, 255, 127, 1000);
  // led_stipe_R.AddTransition(127, 255, 127,  /*->*/   255, 127,   0, 1000);

  led_stipe_R.AddTransition(55, 0,   0,  /*->*/   0,   0, 0, 50);
  led_stipe_R.AddTransition(0,   0,   0,  /*->*/   0,  0, 55, 100);
  led_stipe_R.AddTransition(0,   0, 55,  /*->*/   0, 0, 0, 50);
  led_stipe_R.AddTransition(0,   0,   0,  /*->*/   55,  0, 0, 100);

 
  led_stipe_L.AddTransition(0,   0, 55,  /*->*/   0, 0, 0, 50);
  led_stipe_L.AddTransition(0,   0,   0,  /*->*/   55,  0, 0, 100);
  led_stipe_L.AddTransition(55, 0,   0,  /*->*/   0,   0, 0, 50);
  led_stipe_L.AddTransition(0,   0,   0,  /*->*/   0,  0, 55, 100);

  


  // attach timer to call led update
  timer_leds.attach_ms(25, TickUpdateStipes);
}


//////////////////////////////////////
// loop
void loop()
{
  server.handleClient();
}
