#include "HomeSpan.h"
#include "Blinds.h"
#include "Sensors.h"
#include <GyverOLED.h>
#include <EEPROM.h>
//#include <GyverMotor2.h>

#define OLED_TIMEOUT 50

TaskHandle_t Task0;

//GyverOLED<SSD1306_128x64, OLED_BUFFER, OLED_SPI, 16, 2, 15> oled;  // for ESP32
//GMotor2<DRIVER2WIRE> motor(15, 16);

uint32_t oled_refresh_timer;
uint32_t last_tick_timer;
uint32_t endstop_timer;
uint32_t debounce_timer;
int old_state;
volatile int steps_counter[10];
bool new_data;
bool soft_endstps[10][2];
int max_pos=-1;
int motor_state;
int enc_last_read;
int curr_tick[10];
int target_tick[10];
int old_shade_target[10];
int allow_dir=true;
bool positioning=false;

#define EEPROM_KEY 3
void setup()
{ 
  xTaskCreatePinnedToCore(core0, "Task0", 10000, NULL, 0, &Task0, 0);

  EEPROM.begin(100);

  int read_key;
  EEPROM.get(3, read_key);
  if(read_key!=EEPROM_KEY)
  {
    for(int id=0; id<50; id++)
    {
      EEPROM.put(id, 0);
    }
    EEPROM.put(3, EEPROM_KEY);
    EEPROM.commit();
    max_pos=-1;
  }
  else
  {
    EEPROM.get(10, max_pos);
  }

  

  Serial.begin(115200);
  //motor.reverse(true);

  //oled.init();
  //oled.setContrast(255);

  homeSpan.setPairingCode("06092008");
  homeSpan.begin(Category::Bridges, "ESP32 Arsenii", "ESP32-Arsenii", "Arsenii'sTechnologies");

  homeSpan.setWifiCredentials("i20", "yanatarsnazsof5");

  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();

  // Accessory 3: DHT22 (Temp)
  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();
  new Characteristic::Name("Temperature");
  new TempSensor();

  // Accessory 4: DHT22 (Hum)
  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();
  new Characteristic::Name("Humidity");
  new HumSensor();

  // Accessory 5: Main Shade
  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();
  new Characteristic::Name("Main Shade");
  new WindowShade(3);

  /*
  // Accessory 6: Secondary Shade
  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();
  new Characteristic::Name("Secondary Shade");
  new DEV_WindowShade(1);
  */

  pinMode(26, INPUT_PULLUP);

  pinMode(16, OUTPUT);
  pinMode(27, OUTPUT);

  motor('B');
  positioning=true;
  
  window_shade_state[3]=0;

  last_tick_timer=millis();
  endstop_timer=millis();
  enc_last_read=!digitalRead(26);

  curr_tick[1]=5;
  window_shade_state[3]=0;
  curr_shade_state[1]=0;
}

void loop()
{
  if(Serial.available())
  {
    char inp=Serial.read();
    if(inp=='p')
    {
      Serial.print("curr_tick=");
      Serial.println(curr_tick[1]);
      Serial.print("target_tick=");
      Serial.println(target_tick[1]);
      Serial.print("window_shade_state[3]=");
      Serial.println(window_shade_state[3]);
    }
  }

  if(max_pos!=-1 && !positioning)
  {
    curr_tick[1]=steps_counter[1];//map(curr_shade_state[1], 0, 100, 5, max_pos-5);
    target_tick[1]=map(window_shade_state[3], 0, 100, 10, max_pos-10);

    if(curr_tick[1]-target_tick[1]>0) motor('B');
    else if(curr_tick[1]-target_tick[1]<0) motor('F');

    curr_tick[1]+=motor('G');
    if(motor('G')==-1) 
    {
      if(curr_tick[1]<target_tick[1]) 
      {
        motor('S');
        curr_shade_state[1]=window_shade_state[3];
      }
    }
    else if(motor('G')==1) 
    {
      if(curr_tick[1]>target_tick[1]) 
      {
        motor('S');
        curr_shade_state[1]=window_shade_state[3];
      }
    }
  }

//--

  if(max_pos==-1 || positioning)
  {
    if(millis()-last_tick_timer>=800 && millis()-endstop_timer>=5000)
    {
      if(motor('G')==1)
      {
        soft_endstps[1][1]=true;
        soft_endstps[1][0]=false;
      }    
      else if(motor('G')==-1)
      {
        soft_endstps[1][0]=true;
        soft_endstps[1][1]=false;
      } 
      else
      {
        ;
      }
    }
    //--
    if(soft_endstps[1][0])
    {
      positioning=false;
      soft_endstps[1][0]=false;
      soft_endstps[1][1]=false;
      steps_counter[1]=0;

      window_shade_state[3]=0;
      curr_shade_state[1]=0;
      motor('F');
      delay(500);
      if(max_pos!=-1)
      {
        motor('S');
      }
      //target_shade_state[1]=100;
      endstop_timer=millis();
      curr_tick[1]=0;
      allow_dir=true;
    }
    if(soft_endstps[1][1])
    {
      if(max_pos==-1) max_pos=steps_counter[1];
      EEPROM.put(10, max_pos);
      EEPROM.commit();

      Serial.print("max_pos=");
      Serial.println(max_pos);
      soft_endstps[1][1]=false;
      window_shade_state[3]=100;
      motor('B');
      delay(500);
      motor('S');
      //target_shade_state[1]=100;
      curr_shade_state[1]=100;
      curr_tick[1]=max_pos;
      allow_dir=true;
      positioning=false;
    }
    //--
    if(soft_endstps[1][1]) Serial.println("soft_endstps UP");
    if(soft_endstps[1][0]) Serial.println("soft_endstps DOWN");
  }
  
  //--
  //--
  homeSpan.poll();
}

int motor(char inp)
{
  switch (inp)
  {
    case 'F':  //Forward (open)
      digitalWrite(16, 1);
      digitalWrite(27, 0);

      motor_state = 1;
      inp='A';
    break;
    case 'B':  // Backwards (lower)
      digitalWrite(16, 0);
      digitalWrite(27, 1);

      motor_state = -1;
      inp='A';
    break;
    case 'S':  // Stop
      digitalWrite(16, 0);
      digitalWrite(27, 0);

      motor_state = 0;
      inp='A';
    break;
    case 'G':  //Get state
      return motor_state;
      inp='A';
    break;
  }
  return 15;
}
