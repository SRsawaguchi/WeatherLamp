#include <Arduino.h>
#include <ESP8266WiFi.h>

#define WL_DEBUG
#define BUFFER_SIZE 512

// network
#define SSID "***"
#define PASS "***"
#define CONF_HOST "***"
#define CONF_PATH "/ESP8266/config.txt"
#define HTTP_PORT 80
#define SERIAL_SPEED 115200

// weather leds
#define LED_SUN 12
#define LED_CUL 13
#define LED_RAY 15
#define LED_NET 14
#define SUN 0x01
#define CUL 0x02
#define RAY 0x04
#define NET 0x08

// led status
#define ON HIGH
#define OFF LOW

String g_appid = "";
String g_api_host = "";
String g_api_path = "";
String g_api_city_id = "";
String g_api_cnt = "";
char   g_gp_buff[BUFFER_SIZE] = {0};

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
  byte led_stat = 0;
  String weather_icon = "";
  int weather_id = 0;

  pinMode(LED_SUN, OUTPUT);
  pinMode(LED_CUL, OUTPUT);
  pinMode(LED_RAY, OUTPUT);
  pinMode(LED_NET, OUTPUT);
  
  // Serial Setting
  Serial.begin(SERIAL_SPEED);

  // Wifi Setting
  WiFi.begin(SSID,PASS);
  while(WiFi.status() != WL_CONNECTED){
    delay(100);
    led_stat = led_stat ^ NET;
    led_write(led_stat);
  }
  led_stat |= NET;
  led_write(led_stat);
  
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

    led_write(0);
    return;
  }
  client.print(buildUri(CONF_HOST,CONF_PATH));
  delay(10);

  while(client.available()){
    String line = client.readStringUntil('\r');
    int    eq   = 0;
    line.trim();
    eq = line.indexOf('=');
    if(eq > 0){
      String key  = line.substring(0,eq);
      String val  = line.substring(eq+1);
      
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
      }// end if
    }//end if
  }// end while
  
#ifdef WL_DEBUG
  Serial.println("host  : " + g_api_host);
  Serial.println("path  : " + g_api_path);
  Serial.println("appid : " + g_appid);
  Serial.println("cnt   : " + g_api_cnt);
  Serial.println("id    : " + g_api_city_id);
#endif //WL_DEBUG

  //get weather data
  if( (g_api_host.length() < 5) || (g_api_path.length() < 5)){
    
#ifdef WL_DEBUG
    Serial.println("ERR g_api_host :: " + g_api_host);
    Serial.println("ERR g_api_path :: " + g_api_path);
#endif // WL_DEBUG

    return;
  }//end if

  g_api_host.toCharArray(g_gp_buff,BUFFER_SIZE);
  if (!client.connect(g_gp_buff,HTTP_PORT)){
    
#ifdef WL_DEBUG
    Serial.println(String("connection failed : ") + g_api_host);
#endif // WL_DEBUG

    led_write(0);
    return;
  }//end if

  g_api_path  = g_api_path + "?" +
               "APPID=" + g_appid +
               "&cnt="  + g_api_cnt +
               "&id="   + g_api_city_id;
               
#ifdef WL_DEBUG
  Serial.println("Query : " + g_api_path);
#endif // WL_DEBUG

  client.print(buildUri(g_api_host,g_api_path));
  delay(10);
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
    }// end if
  }// end while

  led_write(0);
  weather_icon.toCharArray(g_gp_buff,BUFFER_SIZE);
  weather_id = atoi(g_gp_buff);
  if(weather_id < sizeof(led_patterns)){
    led_stat |= led_patterns[weather_id];
    led_write(led_stat);
  }// end if
}

void loop(){
}


