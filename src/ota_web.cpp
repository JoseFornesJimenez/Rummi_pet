#include "ota_web.h"
#include <WebServer.h>
#include <Preferences.h>
#include <DNSServer.h>
#include "comm_manager.h"
#include <Update.h>
#include <TFT_eSPI.h>

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
void startAsyncScan() {
  if (!scanInProgress) {
    WiFi.scanNetworks(true);
    scanInProgress = true;
  }
}
void updateScanResults() {
  int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_FAILED || n == WIFI_SCAN_RUNNING) return;
  scannedNetworks = "";
  for (int i = 0; i < n; ++i) {
    scannedNetworks += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
  }
  WiFi.scanDelete();
  scanInProgress = false;
}
String pastelFullPage() {
  updateScanResults();
  String html = "<html><head><title>Rummi_pet Config</title>";
  html += pastelStyle;
  html += "</head><body><div class='card'>";
  html += "<h1>Rummi_pet</h1>";
  html += "<form action='/update' method='POST' enctype='multipart/form-data'>";
  html += "<input type='file' name='update'><br>";
  html += "<input class='btn' type='submit' value='Actualizar firmware'>";
  html += "</form>";
  html += "<form action='/wifi' method='POST'>";
  html += "<select name='ssid'>" + scannedNetworks + "</select><br>";
  html += "<input type='password' name='pass' placeholder='Contraseña'><br>";
  html += "<input class='btn' type='submit' value='Guardar WiFi'>";
  html += "</form>";
  html += "<form action='/resetwifi' method='POST'>";
  html += "<input class='btn' type='submit' value='Borrar configuración WiFi'>";
  html += "</form>";
  html += "<h2>Dispositivos ESP-NOW cercanos</h2>";
  html += "<div class='peer-list'>";
  asyncScanEspNowPeers();
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
  WiFi.begin("REINACASA", "Elpatiodemicasa34"); // Clave correcta
  unsigned long startAttemptTime = millis();
  bool connected = false;
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 8000) {
    delay(200);
  }
  connected = (WiFi.status() == WL_CONNECTED);
  if (!connected) {
    Serial.println("[AP] Intentando activar modo AP...");
    WiFi.disconnect(true, true);
    delay(200);
    WiFi.mode(WIFI_AP);
    delay(200);
    bool apResult = WiFi.softAP("Rummi_pet_AP", NULL, 1); // Sin clave, canal 1
    delay(500);
    Serial.print("[AP] Resultado WiFi.softAP: ");
    Serial.println(apResult ? "OK" : "FALLO");
    Serial.print("[AP] IP: ");
    Serial.println(WiFi.softAPIP());
    dnsServer.start(53, "*", WiFi.softAPIP());
    // Mostrar en pantalla TFT el estado AP
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 30);
    tft.println("Modo AP");
    tft.setCursor(10, 60);
    tft.print("Red: Rummi_pet_AP");
    tft.setCursor(10, 90);
    tft.print("IP: ");
    tft.println(WiFi.softAPIP());
    delay(3000);
  }
  startAsyncScan();
  server.on("/", []() {
    server.send(200, "text/html", pastelFullPage());
  });
  server.on("/wifi", HTTP_POST, []() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
      preferences.begin("wifi", false);
      preferences.putString("ssid", server.arg("ssid"));
      preferences.putString("pass", server.arg("pass"));
      preferences.end();
      server.send(200, "text/plain", "Guardado. Reiniciando...");
      delay(1000);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "Faltan campos");
    }
  });
  server.on("/resetwifi", HTTP_POST, []() {
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();
    server.send(200, "text/plain", "WiFi borrado. Reinicia.");
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", Update.hasError() ? "Fallo al actualizar" : "Actualizado OK. Reiniciando...");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Update.begin();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      Update.end(true);
    }
  });
  // Portal cautivo: redirige todo a la web
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();
}
void handleWebServer() {
  server.handleClient();
}
void drawWiFiStatusIcon() {}
extern TFT_eSPI tft;
