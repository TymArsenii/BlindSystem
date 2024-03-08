#include "HomeSpan.h"
#include "Blinds.h"
#include "Sensors.h"
#include <GyverOLED.h>
//#include <GyverMotor2.h>

#define OLED_TIMEOUT 50


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
int max_pos;
int motor_state;
int enc_last_read;

void setup()
{
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
  
  window_shade_state[3]=0;
  curr_shade_state[1]=50;
  delay(2000);

  last_tick_timer=millis();
  endstop_timer=millis();
  enc_last_read=!digitalRead(26);
}

void loop()
{
  if(enc_last_read==!digitalRead(26))
  {
    enc_last_read=digitalRead(26);
    if(millis()-debounce_timer>=50)
    {
      last_tick_timer=millis();
      soft_endstps[1][1]=false;
      soft_endstps[1][0]=false;
      debounce_timer=millis();
      if(motor('G')==1)
      {
        steps_counter[1]++;
      }
      else if(motor('G')==-1)
      {
        steps_counter[1]--;
      }
      Serial.println(steps_counter[1]);
    }
  }
  /*
  if (millis() - oled_refresh_timer >= OLED_TIMEOUT)
  {
    oled_refresh_timer = millis();
    oled.clear();
    oled.home();
    oled.setScale(1);
    oled.print("Temp: ");
    //oled.print(bme.readTemperature());
    //oled.print(temp);
    oled.setCursor(0, 1);
    oled.print("Hum: ");
    //oled.print(bme.readHumidity());
    oled.setCursor(0, 2);
    oled.print("Main Shade: ");
    oled.print(window_shade_state[3]);
    oled.print("%");
    oled.setCursor(0, 3);
    oled.print("Secondary Shade: ");
    oled.print(window_shade_state[1]);
    oled.print("%");

    oled.update();
  }
*/

  /*
  if(window_shade_state[3]>55 && !soft_endstps[1][1])
  {
    motor.setSpeed(255);
  }
  else if(window_shade_state[3]<45 && !soft_endstps[1][0])
  {
    motor.setSpeed(-255);
  }
  if(window_shade_state[3]>=45 && window_shade_state[3]<=55 || soft_endstps[1][1])
  {
    motor.setSpeed(0);
  }
  */
/*
  if(target_shade_state[1]<window_shade_state[3])
  {
    motor.setSpeed(-255);
  }
  else if(target_shade_state[1]>window_shade_state[3])
  {
    motor.setSpeed(255);
  }
  else if(target_shade_state[1]==window_shade_state[3])
  {
    //motor.setSpeed(0);
  }
*/

  //Serial.print("steps_counter: ");
  //Serial.println(steps_counter[1]);

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
    soft_endstps[1][0]=false;
    soft_endstps[1][1]=false;
    steps_counter[1]=0;

    window_shade_state[3]=100;
    curr_shade_state[1]=0;
    motor('F');
    delay(500);
    //target_shade_state[1]=100;
    endstop_timer=millis();
  }
  if(soft_endstps[1][1])
  {
    max_pos=steps_counter[1];
    Serial.print("max_pos=");
    Serial.println(max_pos);
    soft_endstps[1][1]=false;
    window_shade_state[3]=100;
    motor('B');
    delay(500);
    motor('S');
    //target_shade_state[1]=100;
    curr_shade_state[1]=100;
  }
  //--
  if(soft_endstps[1][1]) Serial.println("soft_endstps UP");
  if(soft_endstps[1][0]) Serial.println("soft_endstps DOWN");
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
