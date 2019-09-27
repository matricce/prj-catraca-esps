#include <Arduino.h>
/*
   v1.0
   Servidor
   WEMOS D1
   /dev/ttyUSB1
   COM10
*/

#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <SSD1306Wire.h>

#define DISPLAY_TITLE "Server v1.0"
#define PEOPLE_MAX 10
//#define PEOPLE_MAX 1000
const char* ssid     = "NodeMCU_AP";
const char* password = "pa$$word";

SSD1306Wire  display(0x3c, D2, D1);
WiFiServer _server(555);//Cria o objeto servidor na porta 555
WiFiClient _client;//Cria o objeto cliente.

void tcp();
void wifiSetup();
void writeLineDisplay(int pixelBeginX, int line, String text);
void setupDisplay();

int peopleCount = 0;
void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando...");
  wifiSetup();
  setupDisplay();
}

uint32_t verificador = 0;

void loop() {
  writeLineDisplay(0, 1, String(peopleCount));
  tcp();//Funçao que gerencia os pacotes e clientes TCP.
}

void wifiSetup() {
  WiFi.mode(WIFI_AP);//Define o WiFi como Acess_Point.
  WiFi.softAP(ssid, password);//Cria a rede de Acess_Point.
  _server.begin();//Inicia o servidor TCP na porta declarada no começo.
}

void tcp() {
  //Detecta se há clientes conectados no servidor.
  if (_client.connected()) {
    //Verifica se o cliente conectado tem dados para serem lidos.
    if (_client.available() > 0) {
      String msg_received = "";

      //Armazena cada Byte (letra/char) na String para formar a mensagem recebida.
      while (_client.available() > 0) {
        char character = _client.read();
        msg_received += character;
      }
      int msg1 = (msg_received.substring(0,1)).toInt();
      String msg2 = msg_received.substring(1);
      //Mostra a mensagem recebida do cliente no Serial Monitor.
      Serial.print("\n...Mensagem do cliente: |" + msg_received + "|");
      if (msg2.substring(1) == "1") {
        int msgToInt =  msg2.toInt();
        if((peopleCount + msgToInt) > 0 && (peopleCount + msgToInt) <= PEOPLE_MAX){
          peopleCount += msgToInt;
          _client.println("pass");
        }
        else if(peopleCount == 0){
          _client.println("empty");
        }
        else if((peopleCount + msgToInt) == 0){
          peopleCount += msgToInt;
          _client.println("empty");
        }
        else
          _client.println("full");
      }
      else{
        _client.println("error");
      }
      writeLineDisplay(0, msg1 + 2, msg2);
    }
  }
  //Se nao houver cliente conectado,
  else {
    _client = _server.available();//Disponabiliza o servidor para o cliente se conectar.
    delay(1);
  }
}

void writeLineDisplay(int pixelBeginX, int line, String text) {
  display.setColor(BLACK);
  uint8_t pixelY = (line * 10) + (6 - line);
  display.fillRect(pixelBeginX, pixelY + 3, 128, 9); //coluna, linha, largura, altura
  display.setColor(WHITE);
  display.drawString(pixelBeginX, pixelY, text);
  display.display();
}

void setupDisplay() {
  display.init();
  display.flipScreenVertically();
  //Apaga o display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  //Seleciona a fonte
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, DISPLAY_TITLE);
  display.setFont(ArialMT_Plain_10);
  for (uint8_t i = 1; i < 6; i++)
    writeLineDisplay(0, i, String(i));
  display.display();
}
