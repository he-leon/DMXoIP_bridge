#ifndef SENSORS_H
#define SENSORS_H

void setupSensors();
void updateSensors();
void handleSensors();

extern float currentTemperature;          // Declare currentTemperature as extern
extern float currentInternalTemperature;  // Declare currentTemperature as extern

#endif  // SENSORS_H
