#include "Arduino.h"
#include "WiFi.h"
#include "esp_https_server.h"
#include "FastLED.h"

#define LED_TYPE      WS2812      
#define COLOR_ORDER   GRB
#define max_bright    128
#define NUM_LEDS      8
#define NUM_LEDS_B    64
#define NUM_LINES     8
CRGB leds[NUM_LINES][NUM_LEDS];
CRGB leds_b[NUM_LEDS_B];


typedef enum {
  WARN,
  SAFE,
  OTHER
} board_mode;

typedef enum {
  R,
  Y,
  G
} RYG_mode;

#define RR 0xFF0000
#define YY 0xFFFF00
#define GG 0x00FF00
#define NN 0x000000
#define C1 0xCC00CC

board_mode boardLed = SAFE;
RYG_mode RYGLed = R;

#define RYG_delay 5000
#define LED_PIN_1       15
#define LED_PIN_2       2
#define LED_PIN_3       0
#define LED_PIN_4       4
#define LED_PIN_5       16
#define LED_PIN_6       17
#define LED_PIN_7       5
#define LED_PIN_8       18
#define LED_PIN_9_B     32

#define LED_RYG_PIN_R1  19  
#define LED_RYG_PIN_Y1  21  
#define LED_RYG_PIN_G1  22  

#define LED_RYG_PIN_R2  23
#define LED_RYG_PIN_Y2  33
#define LED_RYG_PIN_G2  25


int LED_RYG_PIN[2][3] = {
  {LED_RYG_PIN_R1,LED_RYG_PIN_Y1,LED_RYG_PIN_G1},
  {LED_RYG_PIN_R2,LED_RYG_PIN_Y2,LED_RYG_PIN_G2}
};
#define RYGs_NUM 2
#define RYG_LED_NUM 3

#define SSID      "A8"
#define PASSWORD  "123456789"
//192.168.10.120

httpd_handle_t stream_httpd = NULL;

CRGB lines_color_set[NUM_LEDS][NUM_LINES];
String lines_cmd[NUM_LINES][2];


void lines_function(void run(size_t i, size_t j)){
  for (size_t i=0 ; i<NUM_LINES ; i++){
    for (size_t j=0 ; j<NUM_LEDS ; j++){
      run(i, j);
    }
  }
}
void WRITE_LINES_RYG(size_t i, size_t j){
  switch (RYGLed)
  {
  case R:
    if(i==4)      lines_color_set[i][j] = CRGB(RR);
    else if(i==6) lines_color_set[i][j] = CRGB(RR);
    break;
  case Y:
    if(i==4)      lines_color_set[i][j] = CRGB(YY);
    else if(i==6) lines_color_set[i][j] = CRGB(YY);
    break;
  case G:
    if(i==4)      lines_color_set[i][j] = CRGB(GG);
    else if(i==6) lines_color_set[i][j] = CRGB(GG);
    break;
  }

}
void WRITE_LINES_200_0_200 (size_t i, size_t j) {
  leds[i][j] = CRGB(C1);
  FastLED.show();
}
void WRITE_LINES_0_0_0 (size_t i, size_t j) {
  leds[i][j] = CRGB(NN);
  FastLED.show();

}
void WRITE_LINES_CMD (size_t i, size_t j) {
  lines_color_set[i][j] = (lines_cmd[i][1]=="1") ? CRGB(RR):CRGB(NN);
  leds[i][j] = lines_color_set[i][j];
  FastLED.show();
}

static esp_err_t lines_handler(httpd_req_t *req){
  char resp_str[100];
  httpd_req_get_url_query_str(req, resp_str, sizeof(resp_str));
 
  String split_str = String(resp_str);
  int split_val;  
  int data_v = 0;

  do
  {
    split_val = split_str.indexOf("&");
    String str = split_str.substring(0,split_val);
    split_str = split_str.substring(split_val+1, -1);

    int i = str.indexOf("=");
    String key = str.substring(0, i);
    String val = str.substring(i+1, -1);

    lines_cmd[data_v][0] = key;
    lines_cmd[data_v][1] = val;

    data_v++;
    
  } while (split_val != -1);

  lines_function(WRITE_LINES_CMD);

  yield();


  return httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t board_handler(httpd_req_t *req){
  char resp_str[100];
  httpd_req_get_url_query_str(req, resp_str, sizeof(resp_str));
 
  if(!strcmp(resp_str, "warn"))         boardLed = WARN;
  else if (!strcmp(resp_str, "safe"))   boardLed = SAFE; 
  else                                  boardLed = OTHER;

  switch (boardLed)
  {
  case WARN:
    for (size_t i=0; i<NUM_LEDS_B ; i++){
      leds_b[i] = CRGB(RR);
      FastLED.show();
    }
    break;
  case SAFE:
    for (size_t i=0; i<NUM_LEDS_B ; i++){
      leds_b[i] = CRGB(GG);
      FastLED.show();
    }
    break;
  case OTHER:
    for (size_t i=0; i<NUM_LEDS_B ; i++){
      leds_b[i] = CRGB(NN);
      FastLED.show();
    }
    break;
  }


  return httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
}

void RYG_function(void run(size_t i, size_t j)){
  for (size_t i=0; i < RYGs_NUM ; i++){
    for (size_t j=0; j < RYG_LED_NUM ; j++) {
      run(i,j);
    }
  }
}
void SET_RYG (size_t i, size_t j){
  pinMode(LED_RYG_PIN[i][j], OUTPUT);
}
void WRITE_RYG (size_t i, size_t j){
  int arr[3] = {0,0,0};
  arr[RYGLed] = 1;
  digitalWrite(LED_RYG_PIN[i][j], arr[j]);
}

void LED_Task(void *pvParam){
  RYG_function(SET_RYG);
  
  while (1){

    switch (RYGLed)
    {
    case R:
      RYGLed = G;
      break;
    case Y:
      RYGLed = R;
      break;
    case G:
      RYGLed = Y;
      break;
    }
    RYG_function(WRITE_RYG);

    vTaskDelay(RYG_delay);
  }
}



void setup() {
  disableCore0WDT();
  Serial.begin(115200);

  WiFi.begin(SSID, PASSWORD);
  while(!WiFi.isConnected()){
    Serial.println(".");
    delay(100);
  }
  
  Serial.println(WiFi.localIP());

  FastLED.setBrightness(max_bright);    
  LEDS.addLeds<LED_TYPE, LED_PIN_9_B, COLOR_ORDER>(leds_b,NUM_LEDS_B);
  for (size_t i=0; i<NUM_LEDS_B ; i++){
    leds_b[i] = CRGB(C1);
    FastLED.show();
  }
  vTaskDelay(1000);
  for (size_t i=0; i<NUM_LEDS_B ; i++){
    leds_b[i] = CRGB(NN);
    FastLED.show();
  }

  LEDS.addLeds<LED_TYPE, LED_PIN_1, COLOR_ORDER>(leds[0],NUM_LEDS);
  LEDS.addLeds<LED_TYPE, LED_PIN_2, COLOR_ORDER>(leds[1],NUM_LEDS);
  LEDS.addLeds<LED_TYPE, LED_PIN_3, COLOR_ORDER>(leds[2],NUM_LEDS);
  LEDS.addLeds<LED_TYPE, LED_PIN_4, COLOR_ORDER>(leds[3],NUM_LEDS);
  LEDS.addLeds<LED_TYPE, LED_PIN_5, COLOR_ORDER>(leds[4],NUM_LEDS);
  LEDS.addLeds<LED_TYPE, LED_PIN_6, COLOR_ORDER>(leds[5],NUM_LEDS);
  LEDS.addLeds<LED_TYPE, LED_PIN_7, COLOR_ORDER>(leds[6],NUM_LEDS);
  LEDS.addLeds<LED_TYPE, LED_PIN_8, COLOR_ORDER>(leds[7],NUM_LEDS);
  FastLED.setBrightness(max_bright); 

  lines_function(WRITE_LINES_200_0_200);
  vTaskDelay(1000);
  lines_function(WRITE_LINES_0_0_0);

  httpd_config_t config_http = HTTPD_DEFAULT_CONFIG();
  config_http.server_port = 80;

  httpd_uri_t lines_uri = {
            .uri      = "/lines",
            .method   = HTTP_GET,
            .handler  = lines_handler,
            .user_ctx = NULL 
  };

  httpd_uri_t board_uri = {
            .uri      = "/board",
            .method   = HTTP_GET,
            .handler  = board_handler,
            .user_ctx = NULL
  };

  if(httpd_start(&stream_httpd, &config_http) == ESP_OK){
    httpd_register_uri_handler(stream_httpd, &lines_uri);
    httpd_register_uri_handler(stream_httpd, &board_uri);
   
  }
  
 xTaskCreatePinnedToCore(LED_Task, "RYG_Task", 1000, NULL, 1, NULL, 1);


}

void loop() {

}