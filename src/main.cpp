#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>  


const char* ssid = "REDE-WIFI";
const char* password = "SENHA-WIFI";


WebServer server(80);


#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


Servo servo;               
#define SERVOPIN 27        
const int servoOnPos = 180; 
const int servoOffPos = 0;  
bool motorAcionado = false; 
bool oscilarMotor = false;  
unsigned long lastOscillationTime = 0; 
const int oscillationInterval = 1000;   

const float tempThreshold = 28.0;  

String getPage() {
  String motorState = motorAcionado ? "O motor está acionado" : "O motor está desativado";
  
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Monitor de Temperatura e Umidade</title>";
  html += "<style>body{background-color:#121212;color:#ffffff;font-family:Arial, sans-serif;text-align:center;margin-top:50px;}";
  html += "h1{color:#BB86FC;}p{font-size:1.5rem;margin:10px;}#container{display:inline-block;padding:20px;border-radius:10px;background-color:#1F1B24;box-shadow:0 4px 8px rgba(0,0,0,0.5);}";
  html += "#temp, #humidity{font-size:2rem;font-weight:bold;color:#03DAC5;}";
  html += "#motor{font-size:1.5rem;color:" + String(motorAcionado ? "#03DAC5" : "#CF6679") + ";font-weight:bold;}</style>";
  html += "<script>setInterval(function(){getData();}, 2000);function getData(){var xhttp=new XMLHttpRequest();xhttp.onreadystatechange=function(){if(this.readyState==4&&this.status==200){var data=JSON.parse(this.responseText);document.getElementById('temp').innerHTML=data.temperature+' &#8451;';document.getElementById('humidity').innerHTML=data.humidity+' %';document.getElementById('motor').innerHTML=data.motorState;}};xhttp.open('GET', '/data', true);xhttp.send();}</script>";
  html += "</head><body>";
  html += "<h1>Monitor de Temperatura e Umidade</h1>";
  html += "<div id='container'>";
  html += "<p>Temperatura: <span id='temp'>-- &#8451;</span></p>";
  html += "<p>Humidade: <span id='humidity'>-- %</span></p>";
  html += "<p id='motor'>" + motorState + "</p>";
  html += "</div></body></html>";
  return html;
}

void controlaServo(float temperatura) {
  if (temperatura > tempThreshold) {
    if (!oscilarMotor) {
      oscilarMotor = true;
      motorAcionado = true;
      Serial.println("Motor acionado e começando a oscilar.");
    }
  } else {
    if (oscilarMotor) {
      oscilarMotor = false;
      servo.write(servoOffPos);
      motorAcionado = false;
      Serial.println("Motor desativado e oscilação parada.");
    }
  }

  if (oscilarMotor) {
    unsigned long currentTime = millis();
    if (currentTime - lastOscillationTime >= oscillationInterval) {
      static bool oscilarParaDireita = true;
      if (oscilarParaDireita) {
        servo.write(servoOnPos);  
        Serial.println("Motor para a direita (180 graus).");
      } else {
        servo.write(servoOffPos);  
        Serial.println("Motor para a esquerda (0 graus).");
      }
      oscilarParaDireita = !oscilarParaDireita;  
      lastOscillationTime = currentTime;  
    }
  }
}

void handleData() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temp) || isnan(humidity)) {
    server.send(500, "text/plain", "Erro ao ler o sensor DHT");
    return;
  }

  controlaServo(temp);

  // Monta a resposta JSON
  String json = "{\"temperature\": " + String(temp, 1) + ", \"humidity\": " + String(humidity, 1) + ", \"motorState\": \"" + (motorAcionado ? "O motor está acionado" : "O motor está desativado") + "\"}";
  server.send(200, "application/json", json);  
}

void handleRoot() {
  String html = getPage();
  server.send(200, "text/html", html);  
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  delay(2000);

  servo.attach(SERVOPIN);
  servo.write(servoOffPos);  

  // Conectando ao Wi-Fi
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectado ao Wi-Fi!");

  Serial.println("Acesse a página web em: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/data", handleData);

  server.begin();
  Serial.println("Servidor web iniciado!");
}

void loop() {
  server.handleClient();
}
