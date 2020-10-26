/**
 * @author Dominik Pająk (onir) <onir.redne@mdrk.net>
 * @copyright 2020 Dominik Pająk
 * @license GPLv3
 * 
 * 
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
#include "savedcolors.h"

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
const char content_json[] = "application/json";
ESP8266WebServer server(80);

////////////////////////////////////////
//LED Connections
const int led_A_Red = 5;    // D1 
const int led_A_Green = 4; // D2
const int led_A_Blue = 16; // D0

const int led_B_Red = 14;    // D5
const int led_B_Green = 12; // D6
const int led_B_Blue = 0; // D3

const int led_C_Red = 15;    // D8 
const int led_C_Green = 3; // RX
const int led_C_Blue = 13; // D7



LedStripeCtl led_stipe_A = LedStripeCtl(led_A_Red, led_A_Green, led_A_Blue, PWM_DUTY_CYCLE);
LedStripeCtl led_stipe_B = LedStripeCtl(led_B_Red, led_B_Green, led_B_Blue, PWM_DUTY_CYCLE);
LedStripeCtl led_stipe_C = LedStripeCtl(led_C_Red, led_C_Green, led_C_Blue, PWM_DUTY_CYCLE);
LedStripeCtl * led_stripes[] = {&led_stipe_A, &led_stipe_B, &led_stipe_C};
Ticker timer_leds = Ticker();
SavedColors saved_colors = SavedColors(SAVED_COLORS_FILE);

bool led_power = false;
uint16_t power_off_timer = 0;
uint16_t color_peek_timer = 0;

Ticker timer_clock = Ticker();

////////////////////////////////////////
// global led updater
void TickUpdateStipes(void)
{
  led_stipe_A.Update();
  led_stipe_B.Update();
  led_stipe_C.Update();
}

////////////////////////////////////////
// internal clock
void TickClock(void)
{
  if(power_off_timer)
  {
    power_off_timer--;
    if(power_off_timer == 0)
    {
      led_power = !led_power;
      for(LedStripeCtl * sctl : led_stripes) 
        sctl->SetPower(led_power);
    }
  }

  // automatically swich back to base color or transfer after peek
  if(color_peek_timer)
  {
    color_peek_timer--;
    if(power_off_timer == 0)
    {
      for(LedStripeCtl * sctl : led_stripes) 
        sctl->Switch2BaseColor();
    }
  }
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


////////////////////////////////////////////////
// AJAX Set color
void WebAjaxPeekColorHandler()
{
  Serial.printf("AJAX: %s", server.uri().c_str());
  int r = atoi(server.arg("r").c_str());
  int g = atoi(server.arg("g").c_str());
  int b = atoi(server.arg("b").c_str());

  for(LedStripeCtl * sctl : led_stripes) 
  {
    if(!sctl->ColorPeekMode())
      sctl->Switch2PeekColor();
    sctl->SetColor(r, g, b);
  }

  server.send(200, content_plain);
  Serial.println(" - 200");
  color_peek_timer = DEFAULT_PEEK_COLOR_TOUT; // set timeout to restore default
}

void WebAjaxUnsetPeekColorHandler()
{
  Serial.printf("AJAX: %s", server.uri().c_str());
  for(LedStripeCtl * sctl : led_stripes) 
  {
    sctl->Switch2BaseColor();
  }
  color_peek_timer = 0; // disable timeout

  server.send(200, content_plain);
  Serial.println(" - 200");
}

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
}


////////////////////////////////////////////////
// AJAX Read general status (stripes and timers)
void WebAjaxGetStripesStateHandler(void)
{
  char json_buffer[sizeof(led_stripes) * JSON_STRIPE_STATE_MAX + 32] = {0};
  int8_t i = 0;
  uint16_t offset = 0;
  const char * state = nullptr;
  Serial.printf("AJAX: %s", server.uri().c_str());

  offset += sprintf(json_buffer + offset, "{\"power\":%d,\"timer\":%u, \"strp\":[", led_power, power_off_timer);

  for(LedStripeCtl * sctl : led_stripes) 
  {
    LedStripeState &cc = sctl->GetColorState();
    uint8_t r = cc.GetColor_R();
    uint8_t g = cc.GetColor_G();
    uint8_t b = cc.GetColor_B();
    state = sctl->GetStateStr();
    offset += sprintf(json_buffer + offset, "{\"stripe\":%u,\"r\":%u,\"g\":%u,\"b\":%u,\"state\":\"%s\"},", i, r, g, b, state);
  }
  json_buffer[offset-1] = ']';
  json_buffer[offset] = '}';
  offset++;

  server.send(200, content_json, json_buffer);
  Serial.print(json_buffer);
  Serial.println(" - 200");
}


////////////////////////////////////////////////
// AJAX Manage favorite colors
void WebAjaxSavedColorsSet(void)
{
  Serial.printf("AJAX: %s", server.uri().c_str());
  uint8_t r = atoi(server.arg("r").c_str());
  uint8_t g = atoi(server.arg("g").c_str());
  uint8_t b = atoi(server.arg("b").c_str());

  if(server.arg("id").length() > 0)
  {
    uint8_t id = atoi(server.arg("id").c_str());
    saved_colors.SetColor(id, r, g, b, server.arg("name").c_str());
  }
  else
  {
    saved_colors.AddColor(r, g, b, server.arg("name").c_str());
  }
  server.send(200, content_plain);
  Serial.println(" - 200");
}

void WebAjaxSavedColorsGet(void)
{
  Serial.printf("AJAX: %s", server.uri().c_str());
  char json_buffer[JSON_SAVED_COLOR_ENTRY * MAX_SAVED_COLORS] = {0};
  uint16_t offset = 0;
  uint8_t r = 0, g = 0, b = 0, set = 0, id = 0, cfree = 0, cmax = 0;
  char name [COLOR_NAME_LEN] = {0};
  cfree = saved_colors.GetFreeColorsCount();
  cmax = saved_colors.GetColorsCount();

  Serial.printf("AJAX: %s", server.uri().c_str());

  offset += sprintf(json_buffer + offset, "{\"max\":%u,\"free\":%u,\"colors\": [", cmax, cfree);
  while(saved_colors.GetNextColor(&r, &g, &b, &set, &id, name))
  {
    name[0] = 0;
    offset += sprintf(json_buffer + offset, "{\"id\":%u,\"r\":%u,\"g\":%u,\"b\":%u,\"set\":%u,\"name\":\"%s\"},", id, r, g, b, set, name);
  }

  if(cmax > cfree > 0)
    offset--;
  
  json_buffer[offset++] = ']';
  json_buffer[offset++] = '}';
  
  server.send(200, content_json, json_buffer);
  Serial.print(json_buffer);
  Serial.println(" - 200");
}

void WebAjaxSavedColorsDel(void)
{

}

void  WebAjaxSavedColorsSync(void)
{
  Serial.printf("AJAX: %s", server.uri().c_str());
  saved_colors.Sync();
  server.send(200, content_plain);
  Serial.println(" - 200");
}

////////////////////////////////////////////////
// AJAX Power
void WebAjaxPowerOffHandler(void)
{
  Serial.printf("AJAX: %s", server.uri().c_str());
  led_power = false;
  for(LedStripeCtl * sctl : led_stripes) 
    sctl->SetPower(led_power);

  server.send(200, content_plain);
  Serial.println(" - 200");
}

void WebAjaxPowerOnHandler(void)
{
  Serial.printf("AJAX: %s", server.uri().c_str());
  led_power = true;
  for(LedStripeCtl * sctl : led_stripes) 
    sctl->SetPower(led_power);

  server.send(200, content_plain);
  Serial.println(" - 200");
}

void WebAjaxPowerTimerHandler(void)
{
  Serial.printf("AJAX: %s", server.uri().c_str());
  power_off_timer = atoi(server.arg("timer").c_str());
  if(power_off_timer > POWER_TIMER_MAX)
    power_off_timer = POWER_TIMER_MAX;
    
  server.send(200, content_plain);
  Serial.println(" - 200");
}


//////////////////////////////////////
// Setup
void setup()
{
  Serial.begin(115200);   //Start serial connection  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  //setup all possible pins to GPIO
  Serial.println("***** Setup all pins to GPIO function...");

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

  Serial.print("***** Loading ledstripes configuration: ");
  saved_colors.Init();
  
 
  Serial.print("***** Settingup web server: ");
  server.on("/ajax/setcolor", WebAjaxSetColorHandler);    // set static color
  server.on("/ajax/setpeek", WebAjaxPeekColorHandler);    //  enable peek mode and set the peek color
  server.on("/ajax/unsetpeek", WebAjaxUnsetPeekColorHandler);    //  stop peek mode
  server.on("/ajax/getstripesstate",  WebAjaxGetStripesStateHandler);    // Get current static colors for all stripes
  server.on("/ajax/savedcolors_set", WebAjaxSavedColorsSet);    // add / set colors to favotites (do not save to flash)
  server.on("/ajax/savedcolors_get", WebAjaxSavedColorsGet);    // get favorite colors
  server.on("/ajax/savedcolors_del", WebAjaxSavedColorsDel);    // delete color from favorites
  server.on("/ajax/savedcolors_sync", WebAjaxSavedColorsSync);  // update flash
  //server.on("/ajax/getconfig", WebAjaxSetColorHandler);   // return configuration
  //server.on("/ajax/saveconfig", WebAjaxSetColorHandler);  // save configuration (try to cache it to limit flash writes)
  //server.on("/ajax/reset", WebAjaxSetColorHandler);       // reboot device
  server.on("/ajax/poweroff", WebAjaxPowerOffHandler);    // turn off
  server.on("/ajax/poweron", WebAjaxPowerOnHandler);     // turn on
    server.on("/ajax/powertimer", WebAjaxPowerTimerHandler);     // turn on
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

  // led_stipe_B.AddTransition(55, 0,   0,  /*->*/   0,   0, 0, 50);
  // led_stipe_B.AddTransition(0,   0,   0,  /*->*/   0,  0, 55, 100);
  // led_stipe_B.AddTransition(0,   0, 55,  /*->*/   0, 0, 0, 50);
  // led_stipe_B.AddTransition(0,   0,   0,  /*->*/   55,  0, 0, 100);

 
  // led_stipe_A.AddTransition(0,   0, 55,  /*->*/   0, 0, 0, 50);
  // led_stipe_A.AddTransition(0,   0,   0,  /*->*/   55,  0, 0, 100);
  // led_stipe_A.AddTransition(55, 0,   0,  /*->*/   0,   0, 0, 50);
  // led_stipe_A.AddTransition(0,   0,   0,  /*->*/   0,  0, 55, 100);

  led_stipe_A.AddTransition(0,   0,   0,  /*->*/   255, 255, 255, 500);
  led_stipe_A.AddTransition(255, 255, 255,  /*->*/   0,  0, 0, 500);

  led_stipe_B.AddTransition(0,   0,   0,  /*->*/   255, 255, 255, 500);
  led_stipe_B.AddTransition(255, 255, 255,  /*->*/   0,  0, 0, 500);

  led_stipe_C.AddTransition(0,   0,   0,  /*->*/   255, 255, 255, 500);
  led_stipe_C.AddTransition(255, 255, 255,  /*->*/   0,  0, 0, 500);



  // attach timer to call led update
  timer_leds.attach_ms(25, TickUpdateStipes);
  // attach interna clock timer
  timer_clock.attach_ms(1000, TickClock);
}


//////////////////////////////////////
// loop
void loop()
{
  server.handleClient();
}
