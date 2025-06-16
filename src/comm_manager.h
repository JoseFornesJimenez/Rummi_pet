#pragma once
#include <Arduino.h>
#include <vector>
struct EspNowPeer {
  uint8_t mac[6];
  String macStr;
};
class CommManager {
public:
  void begin();
  void loop();
  int getMode();
  void sendMessage(const String& msg);
};
void asyncScanEspNowPeers();
String getPeersJson();
extern std::vector<EspNowPeer> foundPeers;
