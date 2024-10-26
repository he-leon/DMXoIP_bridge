#ifndef WIFICONFIG_H
#define WIFICONFIG_H

struct WiFiConfig
{
  String ssid;
  String password;
  int priority;  // Optional: Higher priority networks connect first
};

#endif  // WIFICONFIG_H
