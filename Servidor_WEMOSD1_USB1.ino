#include <Arduino.h>
/*
   v1.1
   Servidor
   WEMOS D1
   /dev/ttyUSB1
   COM10
*/

#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <SSD1306Wire.h>

#define DISPLAY_TITLE "Server v1.1"
#define MAX_CLIENTS 3
#define PEOPLE_MAX 10
//#define PEOPLE_MAX 1000
const char* ssid     = "NodeMCU_AP";
const char* password = "pa$$word";

SSD1306Wire  display(0x3c, D2, D1);
WiFiServer _server(555);//Cria o objeto servidor na porta 555
WiFiClient _clients[MAX_CLIENTS];//Cria o objeto cliente.

void tcp();
void tcp2();
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
  if (_server.hasClient()) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (!_clients[i] || !_clients[i].connected()) {
        if (_clients[i])
          _clients[i].stop();
        _clients[i] = _server.available();
        Serial.print("New Client : ");
        Serial.print(String(i + 1) + " - ");
        continue;
      }
    }
    WiFiClient _clients = _server.available();
    _clients.stop();
  }
  for (int i = 0; i < MAX_CLIENTS; i++) {
    //Detecta se há clientes conectados no servidor.
    if (_clients[i] && _clients[i].connected()) {
      //Verifica se o cliente conectado tem dados para serem lidos.
      if (_clients[i].available() > 0) {
        String msg_received = "";

        //Armazena cada Byte (letra/char) na String para formar a mensagem recebida.
        while (_clients[i].available() > 0) {
          char character = _clients[i].read();
          msg_received += character;
        }
        int msg1 = (msg_received.substring(0, 1)).toInt();
        String msg2 = msg_received.substring(1);
        //Mostra a mensagem recebida do cliente no Serial Monitor.
        Serial.print("\n...Mensagem do cliente: |" + msg_received + "|");
        if (msg2.substring(1) == "1") {
          int msgToInt =  msg2.toInt();
          if ((peopleCount + msgToInt) > 0 && (peopleCount + msgToInt) <= PEOPLE_MAX) {
            peopleCount += msgToInt;
            _clients[i].println("pass");
          }
          else if (peopleCount == 0) {
            _clients[i].println("empty");
          }
          else if ((peopleCount + msgToInt) == 0) {
            peopleCount += msgToInt;
            _clients[i].println("empty");
          }
          else
            _clients[i].println("full");
          writeLineDisplay(0, msg1 + 2, msg2);
        }
        else if(msg_received == "Conectado")
          _clients[i].println("Cliente conectado");
        else if(msg1 == 4)
          _clients[i].println("Pessoas dentro do local: " + String(peopleCount) + "/" + PEOPLE_MAX);
        else
          _clients[i].println("error");
      }
    }
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
  display.display();
}
