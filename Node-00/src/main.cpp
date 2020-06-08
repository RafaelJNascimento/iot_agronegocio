#include <Arduino.h>
#include <ESP8266WiFi.h>        // Biblioteca NodeMCU wifi
#include <RCSwitch.h>           // Biblioteca NodeMCU para RF
#include <Servo.h>              // Biblioteca NodeMCU para Servo Motor
#include <BlynkSimpleEsp8266.h> // Biblioteca NodeMCU para Blynk

//Definições de Constantes

//Constantes sonoras
#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880

//Constantes de pinos
#define SER_PIN 12  //D6
#define RES_PIN 15  //D8
#define TEMP_PIN A0 //Pino Analógico do NodeMCU
int SPK_PIN = 13;   //D7

//Definição de constantes login e senha do wifi
const char *ssid = "Doentes";
const char *pass = "maisdoentesainda";

//Chave Blynk
char auth[] = "d718b748b3be4f46a6351984f3089f04";

//Chave Thingspeak
char host[] = "api.thingspeak.com";
String ApiKey = "J5YBBOBQSTQR0SIZ";
String caminho = "/update?key=" + ApiKey + "&field2=";
long previousMillis = 0;           // Variável de controle do tempo
long tempo = 20000;                // Tempo em ms do intervalo a ser executado
boolean enviar = false;            // Variável para controle do tempo de envio para o thingspeak
boolean alarme = false;            // Variável para controle de alarme
int valor_temperatura = 0;         // Início da variável de cálculo para envio
Servo meuServo;                    // Variável para servo motor do portão
RCSwitch meuReceptor = RCSwitch(); // Variável para o receptor RF

void abrePortao();
void enviaThingspeak();
void alarmeSonoro();

void setup()
{

  Serial.begin(115200);
  meuServo.attach(SER_PIN);           // Seta porta para servo
  meuReceptor.enableReceive(RES_PIN); // Seta porta para receptor
  Serial.println("");
  Serial.print("Conectando com ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass); // Configuração do wifi

  // Enquanto tenta conectar
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado com ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Blynk.config(auth); // Configuração da chave para Blynk
}

// Função para ativar o alarme
BLYNK_WRITE(V0)
{
  int i = param.asInt();
  if (i == 1)
  {
    Serial.println("Alarme Ligado");
    alarme = true;
  }
  else
  {
    alarme = false;
    Serial.println("Alarme Desligado");
  }
}

// Função para abrir o portão via celular
BLYNK_WRITE(V2)
{
  int i = param.asInt();
  if (i == 1)
    abrePortao();
}

void loop()
{
  valor_temperatura = (float(analogRead(TEMP_PIN)) * 3.3 / 1023) * 100; // Cálculo para precisão da leitura
  Blynk.run();                                                          // Inicia serviço Blynk
  Blynk.virtualWrite(V1, valor_temperatura);                            // Escreve o valor da temperatura na porta virtual do Blynk
  enviaThingspeak();                                                    // Chama a função de envio do Thingspeak

  // se os dados estiverem disponíveis
  if (meuReceptor.available())
  {

    // mostra os dados recebidos
    Serial.print("Valor recebido: ");
    Serial.println(meuReceptor.getReceivedValue());
    Serial.print("Tamanho em bit's: ");
    Serial.println(meuReceptor.getReceivedBitlength());

    // isso deve ser chamado logo após receber dados
    meuReceptor.resetAvailable();
    abrePortao();
  }
  while (alarme)
  { // soar alarme
    alarmeSonoro();
  }
}

// Função para enviar os dados para Thingspeak
void enviaThingspeak()
{
  unsigned long currentMillis = millis(); //Tempo atual em ms

  //Lógica de verificação do tempo
  if (currentMillis - previousMillis > tempo)
  {
    previousMillis = currentMillis; // Salva o tempo atual

    //E aqui muda o estado do led
    //verificando como ele estava anteriormente
    if (enviar == false)
    {
      enviar = true;
    }
    else
    {
      enviar = false;
    }

    if (enviar)
    {
      WiFiClient client;
      const int httpPort = 80;
      if (!client.connect(host, httpPort))
      {
        Serial.println("connection failed");
        return;
      }
      client.print(String("GET ") + caminho + valor_temperatura + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: keep-alive\r\n\r\n");
      Serial.println("Enviando dados Thingspeak");
    }
  }
}

// Função para abrir o  portão
void abrePortao()
{
  Blynk.notify("O Portão foi Aberto");
  meuServo.write(0);
  delay(5000);
  meuServo.write(80);
  Blynk.notify("O Portão foi Fechado");
}

void beep(unsigned char SPK_PIN, int frequencyInHertz, long timeInMilliseconds)
{
  int x;
  long delayAmount = (long)(1000000 / frequencyInHertz);
  long loopTime = (long)((timeInMilliseconds * 1000) / (delayAmount * 2));
  for (x = 0; x < loopTime; x++)
  {
    digitalWrite(SPK_PIN, HIGH);
    delayMicroseconds(delayAmount);
    digitalWrite(SPK_PIN, LOW);
    delayMicroseconds(delayAmount);
  }
  delay(20);
}

//Função para alarmar
void alarmeSonoro()
{
  //for the sheet music see:
  //http://www.musicnotes.com/sheetmusic/mtd.asp?ppn=MN0016254
  //this is just a translation of said sheet music to frequencies / time in ms
  //used 500 ms for a quart note

  beep(SPK_PIN, a, 500);
  beep(SPK_PIN, a, 500);
  beep(SPK_PIN, a, 500);
  beep(SPK_PIN, f, 350);
  beep(SPK_PIN, cH, 150);

  beep(SPK_PIN, a, 500);
  beep(SPK_PIN, f, 350);
  beep(SPK_PIN, cH, 150);
  beep(SPK_PIN, a, 1000);
  //first bit

  beep(SPK_PIN, eH, 500);
  beep(SPK_PIN, eH, 500);
  beep(SPK_PIN, eH, 500);
  beep(SPK_PIN, fH, 350);
  beep(SPK_PIN, cH, 150);

  beep(SPK_PIN, gS, 500);
  beep(SPK_PIN, f, 350);
  beep(SPK_PIN, cH, 150);
  beep(SPK_PIN, a, 1000);
  //second bit...

  beep(SPK_PIN, aH, 500);
  beep(SPK_PIN, a, 350);
  beep(SPK_PIN, a, 150);
  beep(SPK_PIN, aH, 500);
  beep(SPK_PIN, gSH, 250);
  beep(SPK_PIN, gH, 250);

  beep(SPK_PIN, fSH, 125);
  beep(SPK_PIN, fH, 125);
  beep(SPK_PIN, fSH, 250);
  delay(250);
  beep(SPK_PIN, aS, 250);
  beep(SPK_PIN, dSH, 500);
  beep(SPK_PIN, dH, 250);
  beep(SPK_PIN, cSH, 250);
  //start of the interesting bit

  beep(SPK_PIN, cH, 125);
  beep(SPK_PIN, b, 125);
  beep(SPK_PIN, cH, 250);
  delay(250);
  beep(SPK_PIN, f, 125);
  beep(SPK_PIN, gS, 500);
  beep(SPK_PIN, f, 375);
  beep(SPK_PIN, a, 125);

  beep(SPK_PIN, cH, 500);
  beep(SPK_PIN, a, 375);
  beep(SPK_PIN, cH, 125);
  beep(SPK_PIN, eH, 1000);
  //more interesting stuff (this doesn't quite get it right somehow)

  beep(SPK_PIN, aH, 500);
  beep(SPK_PIN, a, 350);
  beep(SPK_PIN, a, 150);
  beep(SPK_PIN, aH, 500);
  beep(SPK_PIN, gSH, 250);
  beep(SPK_PIN, gH, 250);

  beep(SPK_PIN, fSH, 125);
  beep(SPK_PIN, fH, 125);
  beep(SPK_PIN, fSH, 250);
  delay(250);
  beep(SPK_PIN, aS, 250);
  beep(SPK_PIN, dSH, 500);
  beep(SPK_PIN, dH, 250);
  beep(SPK_PIN, cSH, 250);
  //repeat... repeat

  beep(SPK_PIN, cH, 125);
  beep(SPK_PIN, b, 125);
  beep(SPK_PIN, cH, 250);
  delay(250);
  beep(SPK_PIN, f, 250);
  beep(SPK_PIN, gS, 500);
  beep(SPK_PIN, f, 375);
  beep(SPK_PIN, cH, 125);

  beep(SPK_PIN, a, 500);
  beep(SPK_PIN, f, 375);
  beep(SPK_PIN, c, 125);
  beep(SPK_PIN, a, 1000);
  //and we're done \ó/
}
