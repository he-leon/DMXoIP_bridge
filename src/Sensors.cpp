#include "Sensors.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Arduino.h>

const int oneWirePin = 27;     
float currentTemperature = 0.0; // Variable to store the temperature
float currentInternalTemperature = 0.0; // Variable to store the internal temperature
                                        //
OneWire oneWire(oneWirePin); 

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

long lastUpdate = 0;
const long SENSOR_UPDATE_INTERVAL = 5000; // Update interval in milliseconds

void setupSensors()
{
  sensors.begin();
}

void updateSensors()
{
  sensors.requestTemperatures(); 
}

void handleSensors()
{
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate >= SENSOR_UPDATE_INTERVAL)
  {
    lastUpdate = currentMillis;
    updateSensors();
    currentTemperature = sensors.getTempCByIndex(0);
    Serial.print(currentTemperature);
    Serial.println("ºC");
    currentInternalTemperature = temperatureRead();
    Serial.print(currentInternalTemperature);
    Serial.println("ºC");
  }
}


