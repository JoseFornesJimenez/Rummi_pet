#include "ota_web.h"
#include <WebServer.h>
#include <Preferences.h>
#include <DNSServer.h>
#include "comm_manager.h"
WebServer server(80);
Preferences preferences;
DNSServer dnsServer;
String scannedNetworks = "";
bool scanInProgress = false;
bool playSelectedGIF = false;
int selectedGIFIndex = -1;
extern std::vector<EspNowPeer> foundPeers;
extern String getPeersJson();
const char* pastelStyle = R"(
  <style>
    body { background: #f8fafc; color: #5a5a5a; font-family: 'Segoe UI', sans-serif; }
    .card { background: #fff6f9; border-radius: 18px; box-shadow: 0 2px 8px #e0e0e0; padding: 2em; margin: 2em auto; max-width: 400px; }
    h1 { color: #b48ead; }
    .btn { background: #a3d8f4; color: #fff; border: none; border-radius: 8px; padding: 0.7em 1.5em; font-size: 1em; cursor: pointer; transition: background 0.2s; }
    .btn:hover { background: #b48ead; }
    .peer-list { background: #f3e8ff; border-radius: 10px; padding: 1em; margin-top: 1em; }
    .peer { color: #7b8fa1; font-weight: bold; }
  </style>
)";
String pastelPage() {
  String html = "<html><head><title>Rummi_pet Web</title>";
  html += pastelStyle;
  html += "</head><body><div class='card'>";
  html += "<h1>Rummi_pet</h1>";
  html += "<button class='btn' onclick=\"location.reload()\">Actualizar</button>";
  html += "<h2>Dispositivos ESP-NOW cercanos</h2>";
  html += "<div class='peer-list'>";
  asyncScanEspNowPeers();
  String peers = getPeersJson();
  if (foundPeers.size() == 0) {
    html += "<div>No se detectaron dispositivos.</div>";
  } else {
    for (EspNowPeer& peer : foundPeers) {
      html += "<div class='peer'>" + peer.macStr + "</div>";
    }
  }
  html += "</div>";
  html += "</div></body></html>";
  return html;
}
void setupOTA() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("REINACASA", "Elpatiodemicasa34"); // Conexión predeterminada
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 8000) {
    delay(200);
  }
  server.on("/", []() {
    server.send(200, "text/html", pastelPage());
  });
  server.begin();
}
void handleWebServer() {
  server.handleClient();
}
void drawWiFiStatusIcon() {}
