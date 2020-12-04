/**
 * @author Dominik Pająk (onir) <onir.redne@mdrk.net>
 * @copyright 2020 Dominik Pająk
 * @license GPLv3
 * 
 * 
 */ 
 
#include <inttypes.h>
#include <cstring>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include "config.h"
#include "LittleFS.h"
#include "ledstripestate.h"
#include "ledstripectl.h"
#include "savedcolors.h"
#include "savedtransset.h"

#ifdef ESP32
  #include "spectrum.h"
#endif


////////////////////////////////////////
//SSID and Password of your WiFi router
const char* ssid = "*****";
const char* password = "******";

////////////////////////////////////////
// Web server
const char content_html[] = "text/html; charset=UTF-8";
const char content_html_gzip[] = "gz/html; charset=UTF-8";
const char content_css[]  = "text/css; charset=UTF-8";
const char content_js[]  = "text/javascript; charset=UTF-8";
const char content_plain[] = "text/plain";
const char content_ico[] = "image/vnd.microsoft.icon";
const char content_json[] = "application/json";
ESP8266WebServer server(80);

////////////////////////////////////////
//LED Connections
const int led_A_Red =   PIN_D1;
const int led_A_Green = PIN_D2;
const int led_A_Blue =  PIN_D0;

const int led_B_Red =   PIN_D5;
const int led_B_Green = PIN_D6;
const int led_B_Blue =  PIN_D3;

const int led_C_Red =   PIN_D8;
const int led_C_Green = PIN_RX;
const int led_C_Blue =  PIN_D7;

/////////////////////////////////////////
// Spectrum analyser
#ifdef ESP32
  SpectrumAnalyser spec_analyser = SpectrumAnalyser(PIN_ADC0, SPECTRUM_SAMPLING_FREQ, SPECTRUM_SAMPLES_COUNT);
#endif

///////////////////////////////////////
// led stripes
#ifdef ESP32
  LedStripeCtl led_stipe_A = LedStripeCtl(led_A_Red, led_A_Green, led_A_Blue, PWM_DUTY_CYCLE, &spec_analyser);
  LedStripeCtl led_stipe_B = LedStripeCtl(led_B_Red, led_B_Green, led_B_Blue, PWM_DUTY_CYCLE, &spec_analyser);
  LedStripeCtl led_stipe_C = LedStripeCtl(led_C_Red, led_C_Green, led_C_Blue, PWM_DUTY_CYCLE, &spec_analyser);
#else
  LedStripeCtl led_stipe_A = LedStripeCtl(led_A_Red, led_A_Green, led_A_Blue, PWM_DUTY_CYCLE);
  LedStripeCtl led_stipe_B = LedStripeCtl(led_B_Red, led_B_Green, led_B_Blue, PWM_DUTY_CYCLE);
  LedStripeCtl led_stipe_C = LedStripeCtl(led_C_Red, led_C_Green, led_C_Blue, PWM_DUTY_CYCLE);
#endif

LedStripeCtl * led_stripes[] = {&led_stipe_A, &led_stipe_B, &led_stipe_C};
Ticker timer_leds = Ticker();
SavedColors saved_colors = SavedColors(SAVED_COLORS_FILE);
SavedTransSet saved_transsets = SavedTransSet(SAVED_TRANS_FILE);

////////////////////////////////////////
// Power
bool led_power = false;
uint16_t power_off_timer = 0;
uint16_t color_peek_timer = 0;
Ticker timer_clock = Ticker();

////////////////////////////////////////
//Ticker callbacks
// Led updater
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
  //SPRNTF("ADC: %u\r\n", adc_val);
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
    if(color_peek_timer == 0)
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

  SPRNTF("HTTP: %s", server.uri().c_str()); 

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
      {
        SPRNTLN("Sent less data than expected!");
      }

      file.close();
      SPRNTF(" - 200 [%s]\r\n", p_content);

    }
    else
    {
      // 500
      server.send(500, content_plain, "Internal error");
      SPRNTLN(" - 500");

    }
  }
  else
  {
    // 404
    server.send(404, content_plain, "File not found");
    SPRNTLN(" - 404");
  }
}


////////////////////////////////////////////////
// AJAX Set color
void WebAjaxPeekColorHandler()
{
  //SPRNTF("AJAX: %s", server.uri().c_str());
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
  //SPRNTLN(" - 200");
  color_peek_timer = DEFAULT_PEEK_COLOR_TOUT; // set timeout to restore default
}

void WebAjaxUnsetPeekColorHandler()
{

  SPRNTF("AJAX: %s", server.uri().c_str());

  for(LedStripeCtl * sctl : led_stripes) 
  {
    sctl->Switch2BaseColor();
  }
  color_peek_timer = 0; // disable timeout

  server.send(200, content_plain);
  SPRNTLN(" - 200");
}

void WebAjaxSetColorHandler(void)
{
  //SPRNTF("AJAX: %s", server.uri().c_str());
  int r = atoi(server.arg("r").c_str());
  int g = atoi(server.arg("g").c_str());
  int b = atoi(server.arg("b").c_str());
  int l = atoi(server.arg("l").c_str());

  if(l >= 0 && l <= sizeof(led_stripes) -1)
  {
    led_stripes[l]->SetColor(r, g, b);
    server.send(200, content_plain);
    //SPRNTLN(" - 200");
    return;
  }

  server.send(500, content_plain);
  SPRNTLN(" - 500");
}

void WebAjaxSetTransHandler(void)
{
  SPRNTF("AJAX: %s", server.uri().c_str());
  int l = atoi(server.arg("l").c_str());
  int id = atoi(server.arg("id").c_str());
  uint8_t strp_num = 0, r1 = 0, g1 = 0, b1 = 0, r2 = 0, g2 = 0, b2 = 0;
  uint16_t time = 0;
  
  if(l >= 0 && l <= sizeof(led_stripes) -1)
  {

    SPRNTF("set: %u \r\n", id);
    led_stripes[l]->ClearTransitions();
    while(saved_transsets.GetTransStripe(id, strp_num,  &r1, &g1, &b1, &r2, &g2, &b2, &time))
    {
      led_stripes[l]->AddTransition(r1, g1, b1, r2, g2, b2, time);
      SPRNTF("[%u] %u %u %u -> %u %u %u : %u\r\n", strp_num, r1, g1, b1, r2, g2, b2, time);
      strp_num++;
    }
    
    server.send(200, content_plain);
    SPRNTLN(" - 200");
    return;
  }

  server.send(500, content_plain);
  SPRNTLN(" - 500");
}

////////////////////////////////////////////////
// AJAX Read general status (stripes and timers)
void WebAjaxGetStripesStateHandler(void)
{
  char json_buffer[sizeof(led_stripes) * JSON_STRIPE_STATE_MAX + 32] = {0};
  int8_t i = 0;
  uint8_t sync = 0;
  uint16_t offset = 0;
  const char * state = nullptr;
  SPRNTF("AJAX: %s", server.uri().c_str());

  // chack if flash sync is required
  if(saved_colors.SyncNeeded() || saved_transsets.SyncNeeded())
    sync = 1;

  offset += sprintf(json_buffer + offset, "{\"power\":%d,\"timer\":%u, \"sync\":%u, \"strp\":[", led_power, power_off_timer, sync);

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
  SPRNT(json_buffer);
  SPRNTLN(" - 200");

}


////////////////////////////////////////////////
// AJAX Manage favorite colors
void WebAjaxSavedColorsSet(void)
{

  SPRNTF("AJAX: %s", server.uri().c_str());

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
  SPRNTLN(" - 200");
}

void WebAjaxSavedColorsGet(void)
{
  SPRNTF("AJAX: %s\r\n", server.uri().c_str());
  char json_buffer[JSON_SAVED_COLOR_ENTRY * MAX_SAVED_COLORS] = {0};
  uint16_t offset = 0;
  uint8_t r = 0, g = 0, b = 0, set = 0, id = 0, cfree = 0, cmax = 0;
  char name [COLOR_NAME_LEN] = {0};
  cfree = saved_colors.GetFreeColorsCount();
  cmax = saved_colors.GetColorsCount();

  offset += sprintf(json_buffer + offset, "{\"max\":%u,\"free\":%u,\"colors\": [", cmax, cfree);
  while(saved_colors.GetNextColor(&r, &g, &b, &set, &id, name))
  {
    offset += sprintf(json_buffer + offset, "{\"id\":%u,\"r\":%u,\"g\":%u,\"b\":%u,\"set\":%u,\"name\":\"%s\"},", id, r, g, b, set, name);
    name[0] = 0;
  }

  if(cfree < cmax)
    offset--;
  
  json_buffer[offset++] = ']';
  json_buffer[offset++] = '}';
  
  server.send(200, content_json, json_buffer);
  SPRNT(json_buffer);
  SPRNTLN(" - 200");
}

void WebAjaxSavedColorsDel(void)
{
  SPRNTF("AJAX: %s", server.uri().c_str());
  uint8_t id = atoi(server.arg("id").c_str());
  saved_colors.DelColor(id);
  server.send(200, content_plain);
  SPRNTLN(" - 200");
}

void  WebAjaxSavedSync(void)
{
  SPRNTF("AJAX: %s", server.uri().c_str());
  saved_colors.Sync();
  saved_transsets.Sync();
  server.send(200, content_plain);
  SPRNTLN(" - 200");
}

////////////////////////////////////////////////
// AJAX Transitions
void WebAjaxSavedTransSet(void)
{
  SPRNTF("AJAX: %s\r\n", server.uri().c_str());
  uint8_t stripes[TRANS_STRIPE_RECORD_LEN] = {0};
  uint16_t scount = 0;
  const char * name = server.arg('name').c_str();

  for(scount; scount < MAX_STRIPE_TRANSITIONS; scount++)
  {
    char field[4] = {0};
    sprintf(field, "r%d1", scount);

    uint8_t r1 = atoi(server.arg(field).c_str());
    sprintf(field, "g%d1", scount);
    uint8_t g1 = atoi(server.arg(field).c_str());
    sprintf(field, "b%d1", scount);
    uint8_t b1 = atoi(server.arg(field).c_str());
    sprintf(field, "r%d2", scount);
    uint8_t r2 = atoi(server.arg(field).c_str());
    sprintf(field, "g%d2", scount);
    uint8_t g2 = atoi(server.arg(field).c_str());
    sprintf(field, "b%d2", scount);
    uint8_t b2 = atoi(server.arg(field).c_str());
    sprintf(field, "t%d", scount);
    uint16_t time = atoi(server.arg(field).c_str());
    SavedTransSet::Convert2Stripe(stripes + TRANS_STRIPE_SAVE_BYTES * scount, r1, g1, b1, r2, g2, b2, time);
  }
 
  if(server.arg("id").length() > 0)
  {
    uint8_t id = atoi(server.arg("id").c_str());
    saved_transsets.SetTransSet(id, stripes, scount, server.arg("name").c_str());
  }
  else
  {
    saved_transsets.AddTransSet(stripes, scount, server.arg("name").c_str());
  }
  server.send(200, content_plain);
  SPRNTLN(" - 200");

  //saved_transsets.DumpArray();
}

void WebAjaxSavedTransGet(void)
{
  SPRNTF("AJAX: %s\r\n", server.uri().c_str());
  char json_buffer[JSON_SAVED_TRANS_ENTRY] = {0};
  uint16_t offset = 0;
  uint16_t time = 0;
  uint8_t r1 = 0, g1 = 0, b1 = 0, r2 = 0, g2 = 0, b2 = 0, set = 0, id = 0, sfree = 0, smax = 0;
  char name [TRANS_SET_NAME_LEN] = {0};
  uint8_t stripe[TRANS_STRIPE_RECORD_LEN] = {0};
  sfree = saved_transsets.GetFreeTransSetCount();
  smax = saved_transsets.GetTransSetsCount();

  server.chunkedResponseModeStart(200, content_json);

  offset = sprintf(json_buffer, "{\"max\":%u,\"free\":%u,\"sets\":[", smax, sfree);
  server.sendContent(json_buffer, offset);
  //SPRNTF("chunk [%u B]: %s\r\n", offset, json_buffer);
  memset(json_buffer, 0, sizeof(json_buffer));
  offset = 0;

  while(saved_transsets.GetNextTransSet(stripe, &set, &id, name))
  {
    offset += sprintf(json_buffer + offset, "{\"id\":%u,\"set\":%u,\"name\":\"%s\",\"trans\":[", id, set, name);
    int str_idx = 0;
    for(str_idx; str_idx < MAX_STRIPE_TRANSITIONS; str_idx++) 
    {
      SavedTransSet::ParseStripe(str_idx, stripe, &r1, &g1, &b1, &r2, &g2, &b2, &time);
      if(time == 0)
        break;

     offset += sprintf(json_buffer + offset, "{\"r1\":%u,\"g1\":%u,\"b1\":%u,\"r2\":%u,\"g2\":%u,\"b2\":%u,\"time\":%u},", r1, g1, b1, r2, g2, b2, time);
    }

    if(str_idx)
      offset--;

    offset += sprintf(json_buffer + offset, "]}");
    name[0] = 0;
    server.sendContent(json_buffer, offset);
    //SPRNTF("chunk [%u B]: %s\r\n", offset, json_buffer);
    
    memset(json_buffer, 0, sizeof(json_buffer));
    json_buffer[0] = ',';
    offset = 1;
  }
  offset = 0;
  json_buffer[0] = 0;

  json_buffer[offset++] = ']';
  json_buffer[offset++] = '}';
  server.sendContent(json_buffer, offset);
  //SPRNTF("chunk [%u B]: %s\r\n", offset, json_buffer);
  server.chunkedResponseFinalize();
  SPRNTLN(" - 200");
}

void WebAjaxSavedTransDel(void)
{
  SPRNTF("AJAX: %s", server.uri().c_str());
  uint8_t id = atoi(server.arg("id").c_str());
  saved_transsets.DelTransSet(id);
  server.send(200, content_plain);
  SPRNTLN(" - 200");
}

////////////////////////////////////////////////
// AJAX Power
void WebAjaxPowerOffHandler(void)
{
  SPRNTF("AJAX: %s", server.uri().c_str());
  led_power = false;
  for(LedStripeCtl * sctl : led_stripes) 
    sctl->SetPower(led_power);

  server.send(200, content_plain);
  SPRNTLN(" - 200");
}

void WebAjaxPowerOnHandler(void)
{
  SPRNTF("AJAX: %s", server.uri().c_str());
  led_power = true;
  for(LedStripeCtl * sctl : led_stripes) 
    sctl->SetPower(led_power);

  server.send(200, content_plain);
  SPRNTLN(" - 200");
}

void WebAjaxPowerTimerHandler(void)
{
  SPRNTF("AJAX: %s", server.uri().c_str());
  power_off_timer = atoi(server.arg("timer").c_str());
  if(power_off_timer > POWER_TIMER_MAX)
    power_off_timer = POWER_TIMER_MAX;
    
  server.send(200, content_plain);
  SPRNTLN(" - 200");
}


//////////////////////////////////////
// Setup
void setup()
{
  #ifdef DEBUG_LOG
    Serial.begin(115200);   //Start serial connection  
  #endif
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  SPRNTLN("");

  //setup all possible pins to GPIO
  SPRNTLN("***** Setup all pins to GPIO function...");

  // Wait for connection
  SPRNTF("***** Waiting for WiFi ssid [%s]: ", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    SPRNT(".");
  }

  SPRNTLN("connected");  
  SPRNT("IP address: ");
  SPRNTLN(WiFi.localIP());  //IP address assigned to your ESP

  // init filesystem and do basic checks
  // Get all information of your LittleFS
  SPRNT("***** Inizializing FS: ");

  if (LittleFS.begin())
    SPRNTLN("done.");
  else
    SPRNTLN("fail.");

  FSInfo fs_info;
  LittleFS.info(fs_info);
  SPRNTLN("File sistem info.");
  SPRNT("Total space:      ");
  SPRNT(fs_info.totalBytes);
  SPRNTLN("bytes");
  SPRNT("Total space used: ");
  SPRNT(fs_info.usedBytes);
  SPRNTLN("bytes");
  SPRNT("Block size:       ");
  SPRNT(fs_info.blockSize);
  SPRNTLN("bytes");
  SPRNT("Page size:        ");
  SPRNT(fs_info.pageSize);
  SPRNTLN("bytes");
  SPRNT("Max open files:   ");
  SPRNTLN(fs_info.maxOpenFiles);
  SPRNT("Max path lenght:  ");
  SPRNTLN(fs_info.maxPathLength);
  SPRNTLN();

  SPRNT("***** Loading ledstripes configuration: ");
  saved_colors.Init();
  saved_transsets.Init();
  
 
  SPRNT("***** Settingup web server: ");
  server.on("/ajax/setcolor", WebAjaxSetColorHandler);    // set static color
  server.on("/ajax/settrans", WebAjaxSetTransHandler);    // set static color
  server.on("/ajax/setpeek", WebAjaxPeekColorHandler);    //  enable peek mode and set the peek color
  server.on("/ajax/unsetpeek", WebAjaxUnsetPeekColorHandler);    //  stop peek mode
  
  server.on("/ajax/getstripesstate",  WebAjaxGetStripesStateHandler);    // Get current static colors for all stripes
  server.on("/ajax/savedcolors_set", WebAjaxSavedColorsSet);    // add / set colors to favotites (do not save to flash)
  server.on("/ajax/savedcolors_get", WebAjaxSavedColorsGet);    // get favorite colors
  server.on("/ajax/savedcolors_del", WebAjaxSavedColorsDel);    // delete color from favorites
  server.on("/ajax/saved_sync", WebAjaxSavedSync);  // update flash
    
  server.on("/ajax/poweroff", WebAjaxPowerOffHandler);    // turn off
  server.on("/ajax/poweron", WebAjaxPowerOnHandler);     // turn on
  server.on("/ajax/powertimer", WebAjaxPowerTimerHandler);     // turn on
  
  server.on("/ajax/savedtrans_set", WebAjaxSavedTransSet);    // add / set trans-sets to favotites (do not save to flash)
  server.on("/ajax/savedtrans_get", WebAjaxSavedTransGet);    // get saved trans-sets
  server.on("/ajax/savedtrans_del", WebAjaxSavedTransDel);    // delete trans from favorites

  server.onNotFound(WebRootHandler);  // handle all uri's but ajax
  server.begin();
  SPRNTLN("done");

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
#ifdef ESP32
  spec_analyser.Update();
#endif
}
