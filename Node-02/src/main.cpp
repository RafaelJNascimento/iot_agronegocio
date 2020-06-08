#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
#define BLYNK_PRINT Serial

const char* ssid = "Doentes";
const char* pass = "maisdoentesainda";

#define Luminosidade_pin  A0
#define LuzInterna 13  //D7
#define LuzExterna 15  //D8

//Chave Blynk
char auth[] = "cdd1fab4771741398d10ded9b072ab4d";

//Chave Thingspeak
char host[] = "api.thingspeak.com";
String ApiKey = "J5YBBOBQSTQR0SIZ";
String path = "/update?key=" + ApiKey + "&field3=";
long previousMillis = 0;// Variável de controle do tempo
long tempo = 20000;// Tempo em ms do intervalo a ser executado
boolean enviar = false;
int Luminosidade = 0;

void enviaThingspeak();

void setup() {
  pinMode(LuzInterna,OUTPUT);
  pinMode(LuzExterna,OUTPUT);
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
  pinMode(Luminosidade_pin, INPUT);

}

BLYNK_WRITE(V0) {
    if ( param.asInt() == 1 ) {
      Serial.println("Luz interna Ligada");
      digitalWrite(LuzInterna,HIGH);
    } else {
      Serial.println("Luz interna Apagada");
      digitalWrite(LuzInterna,LOW);
    }
}

BLYNK_WRITE(V2) {
    if ( param.asInt() == 1 ) {
      Serial.println("Luz externa Ligada");
      digitalWrite(LuzExterna,HIGH);
    } else {
      Serial.println("Luz externa Apagada");
      digitalWrite(LuzExterna,LOW);
    }
}

void loop() {
  Luminosidade = (float(analogRead(Luminosidade_pin))* 3.3/1023) * 100;
  Blynk.run();
  Blynk.virtualWrite(V1,Luminosidade);
  enviaThingspeak();
  //Serial.println(Luminosidade);
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
        client.print(String("GET ") + path + Luminosidade + " HTTP/1.1\r\n" +
        "Host: " + host + "\r\n" +
        "Connection: keep-alive\r\n\r\n");
        Serial.println("Enviando dados Thingspeak");
      }
    }
 }
