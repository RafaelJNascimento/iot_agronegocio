#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
#define BLYNK_PRINT Serial

const char* ssid = "Doentes";
const char* pass = "maisdoentesainda";

#define UMIDADE_PIN  A0

//Chave Blynk
char auth[] = "c3f7b291c33c4c61bf0558b5917fd9e7";

//Chave Thingspeak
char host[] = "api.thingspeak.com";
String ApiKey = "J5YBBOBQSTQR0SIZ";
String path = "/update?key=" + ApiKey + "&field1=";
long previousMillis = 0;  // Variável de controle do tempo
long tempo = 20000;       // Tempo em ms do intervalo a ser executado
boolean enviar = false;
int valor_umidade = 0;


void enviaThingspeak();

void setup() {

  Serial.begin(115200);
  Serial.println("");
  Serial.print("Conectando com ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  Blynk.config(auth);

  // Enquanto tenta conectar
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado com ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  pinMode(UMIDADE_PIN, INPUT);

}

void loop() {
  valor_umidade = (float(analogRead(UMIDADE_PIN))* 3.3 / 1023) * 100;
  Blynk.run();
  Blynk.virtualWrite(V1,valor_umidade);
  enviaThingspeak();
}

void enviaThingspeak() {
  unsigned long currentMillis = millis();    //Tempo atual em milisegundos

    //Lógica de verificação do tempo
    if (currentMillis - previousMillis > tempo) {
      previousMillis = currentMillis;    // Salva o tempo atual

      //E aqui muda o estado de envio
      //verificando como ele estava anteriormente
      if (enviar == false) {
        enviar = true;
      } else {
        enviar = false;
      }

      if( enviar ) {
        WiFiClient client;
        const int httpPort = 80;
        if (!client.connect(host, httpPort)) {
          Serial.println("connection failed");
          return;
        }
        client.print(String("GET ") + path + valor_umidade + " HTTP/1.1\r\n" +
        "Host: " + host + "\r\n" +
        "Connection: keep-alive\r\n\r\n");
        Serial.println("Enviando dados Thingspeak");
      }
    }
 }
