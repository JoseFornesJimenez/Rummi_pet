#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("[AP] Iniciando modo AP...");
  WiFi.disconnect(true, true);
  delay(200);
  WiFi.mode(WIFI_AP);
  delay(200);
  bool apResult = WiFi.softAP("ESP32C3_AP", NULL, 1); // Sin clave, canal 1
  delay(500);
  Serial.print("[AP] Resultado WiFi.softAP: ");
  Serial.println(apResult ? "OK" : "FALLO");
  Serial.print("[AP] IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  // No hace nada
}
