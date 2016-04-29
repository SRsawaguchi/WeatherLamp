#define LED_SUN 12
#define LED_CUL 13
#define LED_RAY 15
#define LED_NET 14
#define ON HIGH
#define OFF LOW

int leds[] = {LED_SUN,LED_CUL,LED_RAY,LED_NET};

//アウトプットの設定
void setup()
{
  pinMode(LED_SUN, OUTPUT);
  pinMode(LED_CUL, OUTPUT);
  pinMode(LED_RAY, OUTPUT);
  pinMode(LED_NET, OUTPUT);
}

void loop(){
  int i,pin;

  i = 0;
  while(1){
    pin = leds[i];
    led_all_off();
    led_on(pin);
    delay(1000);
    i = (i + 1) % 4;
  }
}

void led_on(int pin){
  digitalWrite(pin,ON);
}

void led_all_off(void){
  int i,pin;
  for(i=0;i<4;i++){
    pin = leds[i];
    digitalWrite(pin,OFF);
  }
}

