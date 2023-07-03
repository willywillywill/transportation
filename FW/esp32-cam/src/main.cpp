#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// A8: 
//  1: 192.168.89.203
//  2: 192.168.89.176


// http
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
httpd_handle_t stream_httpd = NULL;

#define SSID "A8"
#define PASSWORD "123456789"


// Cmaera
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22



esp_err_t camera_handler(httpd_req_t *req) {
    camera_config_t config_camera;

    config_camera.ledc_channel = LEDC_CHANNEL_0;
    config_camera.ledc_timer = LEDC_TIMER_0;
    config_camera.pin_d0 = Y2_GPIO_NUM;
    config_camera.pin_d1 = Y3_GPIO_NUM;
    config_camera.pin_d2 = Y4_GPIO_NUM;
    config_camera.pin_d3 = Y5_GPIO_NUM;
    config_camera.pin_d4 = Y6_GPIO_NUM;
    config_camera.pin_d5 = Y7_GPIO_NUM;
    config_camera.pin_d6 = Y8_GPIO_NUM;
    config_camera.pin_d7 = Y9_GPIO_NUM;
    config_camera.pin_xclk = XCLK_GPIO_NUM;
    config_camera.pin_pclk = PCLK_GPIO_NUM;
    config_camera.pin_vsync = VSYNC_GPIO_NUM;
    config_camera.pin_href = HREF_GPIO_NUM;
    config_camera.pin_sscb_sda = SIOD_GPIO_NUM;
    config_camera.pin_sscb_scl = SIOC_GPIO_NUM;
    config_camera.pin_pwdn = PWDN_GPIO_NUM;
    config_camera.pin_reset = RESET_GPIO_NUM;
    config_camera.xclk_freq_hz = 20000000;
    config_camera.pixel_format = PIXFORMAT_JPEG;

    if(psramFound()){
        config_camera.frame_size = FRAMESIZE_VGA;
        config_camera.jpeg_quality = 10;
        config_camera.fb_count = 2;
    } else {
        config_camera.frame_size = FRAMESIZE_SVGA;
        config_camera.jpeg_quality = 12;
        config_camera.fb_count = 1;
    }
    esp_err_t err = esp_camera_init(&config_camera);
    if (err != ESP_OK) {
        //Serial.printf("Camera init failed with error 0x%x", err);
        return  ESP_FAIL;
    }

    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    sensor_t * s = esp_camera_sensor_get();
    s->set_vflip(s, 1);
    s->set_framesize(s, FRAMESIZE_VGA);
    s->set_quality(s, 10);
    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            //Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(fb){
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if(res != ESP_OK){
            break;
        }
    }
    return res;
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);


    Serial.begin(115200);

    WiFi.begin(SSID, PASSWORD);

    while(!WiFi.isConnected()){
        Serial.println(".");
        delay(100);
    }
    Serial.println(WiFi.localIP());

    httpd_config_t config_http = HTTPD_DEFAULT_CONFIG();
    config_http.server_port = 80;

    httpd_uri_t camera_uri = {
              .uri      = "/camera",
              .method   = HTTP_GET,
              .handler  = camera_handler,
              .user_ctx = NULL
    };
 
    if (httpd_start(&stream_httpd, &config_http) == ESP_OK) {
          httpd_register_uri_handler(stream_httpd, &camera_uri);
    }
  
}

void loop() {

}

