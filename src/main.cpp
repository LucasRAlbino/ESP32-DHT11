#include <Arduino.h>
#include <WiFi.h>        
#include <WebServer.h>    
#include <DHT.h>           

const char* ssid = "REDE-WIFI";
const char* password = "SENHA-WIFI";

WebServer server(80);

#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

String getPage() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Monitor de Temperatura e Umidade</title>";
  html += "<style>";
  html += "body{font-family:Arial, sans-serif;background-color:#121212;color:#e0e0e0;text-align:center;margin-top:50px;}";
  html += ".container{display:inline-block;background-color:#1e1e1e;padding:20px;border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,0.5);}";
  html += ".data-box{margin:10px;padding:20px;background-color:#292929;border-radius:5px;box-shadow:0 0 5px rgba(0,0,0,0.3);}";
  html += ".data-box p{font-size:20px;margin:5px;}";
  html += "</style>";
  html += "<script>";
  html += "setInterval(function(){getData();}, 2000);"; 
  html += "function getData(){var xhttp = new XMLHttpRequest();";
  html += "xhttp.onreadystatechange = function(){if(this.readyState == 4 && this.status == 200){";
  html += "document.getElementById('temp').innerHTML = this.responseText.split(',')[0] + ' °C';";
  html += "document.getElementById('hum').innerHTML = this.responseText.split(',')[1] + ' %';}};";
  html += "xhttp.open('GET', '/data', true); xhttp.send();}";
  html += "</script>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Monitor de Temperatura e Umidade</h1>";
  html += "<div class='data-box'><p>Temperatura:</p><p id='temp'>Carregando...</p></div>";
  html += "<div class='data-box'><p>Umidade:</p><p id='hum'>Carregando...</p></div>";
  html += "</div>";
  html += "</body></html>";
  return html;
}

void handleData() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temp) || isnan(humidity)) {
    server.send(200, "text/plain", "Erro,Erro");
    return;
  }

  String data = String(temp) + "," + String(humidity);  
  server.send(200, "text/plain", data);
}

void handleRoot() {
  String html = getPage();
  server.send(200, "text/html", html);  
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  delay(2000);

  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectado ao Wi-Fi!");

  Serial.print("Acesse a página em: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  
  server.begin();
  Serial.println("Servidor web iniciado!");
}

void loop() {
  server.handleClient();
}
