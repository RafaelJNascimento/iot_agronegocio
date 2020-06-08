#include <Arduino.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

#define TX_PIN 10

int btn_1 = 9;
int estado_btn_1 = 0;
boolean ultima_leitura_btn_1 = LOW;

void setup() {
  pinMode(btn_1, INPUT);

  Serial.begin(9600);

  // habilitando o trasmissor e setando o pino
  mySwitch.enableTransmit(TX_PIN);
  mySwitch.setPulseLength(500);
  mySwitch.setRepeatTransmit(4);
}

void loop() {
   unsigned long code = 123;
   unsigned int length = 8;
   estado_btn_1 = digitalRead(btn_1);

  if(ultima_leitura_btn_1 != estado_btn_1) {
    delay(3);
    estado_btn_1 = digitalRead(btn_1);

    if(ultima_leitura_btn_1 == LOW && estado_btn_1 == HIGH) {
      mySwitch.send(code, length);
      Serial.println("Dado enviado");
    }
    ultima_leitura_btn_1 = estado_btn_1;
  }
}
