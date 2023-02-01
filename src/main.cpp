#include <WiFi.h>
#include "WebSocketsServer.h"
#include "esp_camera.h"

WebSocketsServer webSocket = WebSocketsServer(81);
WiFiServer server(80);
uint8_t cam_num;
bool connected = false;

String index_html = "<html>\n \
<head>\n \
<title>WS Client</title>\n \
<script src='http://code.jquery.com/jquery-1.9.1.min.js'></script>\n \
</head>\n \
<body>\n \
<div id='output'></div>\n \
</body>\n \
</html>\n \
<script>\n \
jQuery(function($){\n \
if (!('WebSocket' in window)) {\n \
alert('Your browser does not support web sockets');\n \
}else{\n \
setup();\n \
}\n \
function setup(){\n \
var host = 'ws://server_ip:81';\n \
var socket = new WebSocket(host);\n \
if(socket){\n \
socket.onopen = function(){\n \
}\n \
socket.onmessage = function(msg){\n \
showServerResponse(msg.data);\n \
}\n \
socket.onclose = function(){\n \
showServerResponse('The connection has been closed.');\n \
}\n \
}\n \
function showServerResponse(txt){\n \
document.getElementById('output').innerHTML = txt;\n \
}\n \
}\n \
});\n \
</script>";

const char* ssid = "Neo Garbagetown";
const char* password = "gaygaygay";

#define PART_BOUNDARY "123456789000000000000987654321"
#define CAMERA_MODEL_AI_THINKER
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

bool relay = false;
const int RELAY_PIN = 12;

void configCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 9;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void liveCam(uint8_t num){
  //capture a frame
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Frame buffer could not be acquired");
      return;
  }
  //replace this with your own function
  webSocket.sendBIN(num, fb->buf, fb->len);

  //return the frame buffer back to be reused
  esp_camera_fb_return(fb);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            cam_num = num;
            connected = true;
            break;
        case WStype_TEXT:
        case WStype_BIN:
        case WStype_ERROR:      
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("ssid", "password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  pinMode(RELAY_PIN, OUTPUT);
  Serial.println("");
  String IP = WiFi.localIP().toString();
  Serial.print("IP address: " + IP);
  index_html.replace("server_ip", IP);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  configCamera();
}

void http_resp() {
  WiFiClient client = server.available();
  if (client.connected() && client.available()) {
    client.flush();
    client.print(index_html);
    client.stop();
  }
}

void loop() {
  http_resp();
  webSocket.loop();
  if(connected == true){
    liveCam(cam_num);
  }
}


