#include <Arduino.h>
#include <ESP8266WiFi.h>

// system
#define WL_DEBUG
#define SERIAL_SPEED 115200
#define BUFFER_SIZE 512
#define INTERVAL    100   // ms
#define TIMEOUT     5000  // ms

// network
#define SSID "***"
#define PASS "***"
#define CONF_HOST "***"
#define CONF_PATH "/ESP8266/config.txt"
#define HTTP_PORT 80

// leds
#define LED_SUN 12
#define LED_CUL 13
#define LED_RAY 15
#define LED_NET 14
#define SUN 0x01
#define CUL 0x02
#define RAY 0x04
#define NET 0x08

// error codes
#define ERR_WIFI_TIMEOUT 0x01
#define ERR_CONF         0x02
#define ERR_API          0x04

String g_appid = "";
String g_api_host = "";
String g_api_path = "";
String g_api_city_id = "";
String g_api_cnt = "";
char   g_gp_buff[BUFFER_SIZE] = {0};
byte   g_led_stat = 0;
byte   g_error_code = 0;

byte led_patterns[] ={
  0,        // 00 not use
  SUN,      // 01 clear sky
  SUN|CUL,  // 02 few clouds
  CUL,      // 03 scattered clouds
  CUL,      // 04 broken clouds
  0,        // 05 not use
  0,        // 06 not use
  0,        // 07 not use
  0,        // 08 not use
  RAY,      // 09 shower rain
  RAY,      // 10 rain
  CUL|RAY,  // 11 thunderstorm
  0,        // 12 not use
  SUN|RAY,  // 13 snow
};

String buildUri(String host, String path){
  String uri = String("GET ") + path + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close \r\n\r\n";
  delay(10);

  return uri;
}

void led_write(byte leds){
  digitalWrite(LED_SUN,SUN&leds);
  digitalWrite(LED_CUL,CUL&leds);
  digitalWrite(LED_RAY,RAY&leds);
  digitalWrite(LED_NET,NET&leds);
}

//アウトプットの設定
void setup()
{
  String weather_icon = "";
  int weather_id;
  unsigned long timeout;

  pinMode(LED_SUN, OUTPUT);
  pinMode(LED_CUL, OUTPUT);
  pinMode(LED_RAY, OUTPUT);
  pinMode(LED_NET, OUTPUT);
  
  // Serial Setting
  Serial.begin(SERIAL_SPEED);

  // Wifi Setting
  timeout = millis();
  WiFi.begin(SSID,PASS);
  while(WiFi.status() != WL_CONNECTED){
    if(millis() - timeout > TIMEOUT){
      g_error_code |= ERR_WIFI_TIMEOUT;
      
#ifdef WL_DEBUG
      Serial.println("WiFi TIMEOUT");
#endif // WL_DEBUG

      return;
    }
    g_led_stat = g_led_stat ^ NET;
    led_write(g_led_stat);
    delay(INTERVAL);
  }
  g_led_stat |= NET;
  led_write(g_led_stat);
  
#ifdef WL_DEBUG
  Serial.println("Wifi connected.");
  Serial.print("IP address : ");
  Serial.println(WiFi.localIP());
#endif // WL_DEBUG

  // get config file
  WiFiClient client;
  if (!client.connect(CONF_HOST,HTTP_PORT)){
#ifdef WL_DEBUG
    Serial.println(String("connection failed : ") + CONF_HOST);
#endif // WL_DEBUG
    g_error_code |= ERR_CONF;
    return;
  }
  client.print(buildUri(CONF_HOST,CONF_PATH));

  timeout = millis();
  while(client.available() == 0){
    if(millis() - timeout > timeout){
      client.stop();
      g_error_code |= ERR_CONF;
      return;
    }
  }

  while(client.available()){
    String line = client.readStringUntil('\n');
    int    eq   = 0;
    line.trim();
    eq = line.indexOf('=');
    if(eq > 0){
      String key  = line.substring(0,eq);
      String val  = line.substring(eq+1);
      Serial.println("KEY:: "+key);
      if(key.equals("HOST")){
        g_api_host = val;
      }else if(key.equals("PATH")){
        g_api_path = val;
      }else if(key.equals("APPID")){
        g_appid = val;
      }else if(key.equals("cnt")){
        g_api_cnt = val;
      }else if(key.equals("id")){
        g_api_city_id = val;
      }
    }
  }
  
#ifdef WL_DEBUG
  Serial.println("host  : " + g_api_host);
  Serial.println("path  : " + g_api_path);
  Serial.println("appid : " + g_appid);
  Serial.println("cnt   : " + g_api_cnt);
  Serial.println("id    : " + g_api_city_id);
#endif //WL_DEBUG

  //get weather data
  if( (g_api_host.length() < 5) || (g_api_path.length() < 5)){
    g_error_code |= ERR_CONF;
#ifdef WL_DEBUG
    Serial.println("ERR g_api_host :: " + g_api_host);
    Serial.println("ERR g_api_path :: " + g_api_path);
#endif // WL_DEBUG

    return;
  }

  g_api_host.toCharArray(g_gp_buff,BUFFER_SIZE);
  if (!client.connect(g_gp_buff,HTTP_PORT)){
    g_error_code |= ERR_API;
#ifdef WL_DEBUG
    Serial.println(String("connection failed : ") + g_api_host);
#endif // WL_DEBUG

    return;
  }

  g_api_path  = g_api_path + "?" +
               "APPID=" + g_appid +
               "&cnt="  + g_api_cnt +
               "&id="   + g_api_city_id;
               
#ifdef WL_DEBUG
  Serial.println("Query : " + g_api_path);
#endif // WL_DEBUG

  client.print(buildUri(g_api_host,g_api_path));
  timeout = millis();
  while(client.available() == 0){
    if(millis() - timeout > timeout){
      client.stop();
      g_error_code |= ERR_API;
      return;
    }
  }
  while(client.available()){
    String line = client.readStringUntil('\r');
    int idx;
    
    line.trim();
    
#ifdef WL_DEBUG
    Serial.print(line);
#endif // WL_DEBUG

    idx = line.lastIndexOf("icon");
    if(idx >= 0){
      weather_icon = line.substring(idx+7,idx+9);
    }
  }

  weather_icon.toCharArray(g_gp_buff,BUFFER_SIZE);
  weather_id = atoi(g_gp_buff);
  if((0 <= weather_id) && (weather_id < sizeof(led_patterns))){
    if(led_patterns[weather_id]){
      g_led_stat |= led_patterns[weather_id];
    }else{
      g_led_stat |= SUN|CUL|RAY;
    }
  }else{
    g_led_stat |= SUN|CUL|RAY;
  }
  led_write(g_led_stat);
}

// 主にエラー処理。エラー内容に応じてLEDを点滅させる。
void loop(){
  if(g_error_code & ERR_WIFI_TIMEOUT){
    g_led_stat &= ~NET;
    g_led_stat ^= SUN;
  }

  if(g_error_code & ERR_CONF){
    g_led_stat ^= CUL;
  }

  if(g_error_code & ERR_API){
    g_led_stat ^= RAY;
  }
  
  led_write(g_led_stat);
  delay(INTERVAL);
}


