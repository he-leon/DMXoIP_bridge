#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

#include "ConfigParameters.h"

void setupWiFiManager();
void setupOTA();
void handleWiFiManager();
void handleOTA();
void saveConfigCallback();
void setupMenu();
void handleMonitor();
void bindServerCallback();

#endif  // NETWORKCONFIG_H

