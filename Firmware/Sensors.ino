#include <DHT.h>

#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);


struct TempSensor : Service::TemperatureSensor
{
  SpanCharacteristic *temp;

  TempSensor() : Service::TemperatureSensor()
  {
    temp = new Characteristic::CurrentTemperature(-10.0);
    temp->setRange(-50, 100);

    Serial.print("Configuring Temperature Sensor");
    Serial.print("\n");
  }

  void loop()
  {
    if (temp->timeVal() > 3000)
    {
      //float temperature = 35.5;
      float temperature = dht.readTemperature();
      if (isnan(temperature))
      {
        temperature = -50.0;
      }

      temp->setVal(temperature);

      LOG1("Temperature Update: ");
      LOG1(temperature * 9 / 5 + 32);
      LOG1("\n");
    }
  }
};

struct HumSensor : Service::HumiditySensor
{
  SpanCharacteristic *hum;

  HumSensor() : Service::HumiditySensor()
  {
    hum = new Characteristic::CurrentRelativeHumidity(10.0);
    hum->setRange(0, 100);

    Serial.print("Configuring Humidity Sensor");
    Serial.print("\n");
  }

  void loop()
  {
    if (hum->timeVal() > 3000)
    {
      //float humidity = 40.5;
      float humidity = dht.readHumidity();
      if (isnan(humidity))
      {
        humidity = 0.0;
      }

      hum->setVal(humidity);

      LOG1("humidity Update: ");
      LOG1(humidity * 9 / 5 + 32);
      LOG1("\n");
    }
  }
};
/*
struct DEV_PressureSensor : Service::AtmosphericPressureSensor
{
  SpanCharacteristic *pressure;

  DEV_PressureSensor() : Service::AtmosphericPressureSensor{}
  {
    pressure = new Characteristic::CurrentRelativePressure(1000);
    Serial.print("Configuring Air Pressure Sensor");
    Serial.print("\n");

  }  // end constructor

  void loop()
  {
    float press = bme.readPressure();
    if (pressure->timeVal() > 3000)
    {
      pressure->setVal(pressure);
    }
  }
};
*/
