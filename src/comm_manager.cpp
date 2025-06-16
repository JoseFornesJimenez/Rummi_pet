#include "comm_manager.h"
#include <esp_now.h>
#include <WiFi.h>
#include <vector>
std::vector<EspNowPeer> foundPeers;
void asyncScanEspNowPeers() {
    foundPeers.clear();
    int16_t numNetworks = WiFi.scanNetworks(false, true);
    for (int i = 0; i < numNetworks; ++i) {
        if (WiFi.BSSID(i)) {
            EspNowPeer peer;
            memcpy(peer.mac, WiFi.BSSID(i), 6);
            char buf[18];
            sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", peer.mac[0], peer.mac[1], peer.mac[2], peer.mac[3], peer.mac[4], peer.mac[5]);
            peer.macStr = String(buf);
            foundPeers.push_back(peer);
        }
    }
}
String getPeersJson() {
    String json = "[";
    for (size_t i = 0; i < foundPeers.size(); ++i) {
        json += "{\"mac\":\"" + foundPeers[i].macStr + "\"}";
        if (i < foundPeers.size() - 1) json += ",";
    }
    json += "]";
    return json;
}
void CommManager::begin() {}
void CommManager::loop() {}
int CommManager::getMode() { return 0; }
void CommManager::sendMessage(const String& msg) {}
