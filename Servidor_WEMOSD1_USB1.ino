#include <Arduino.h>
/*
   v1.1.1.5
   Servidor
   WEMOS D1
   /dev/ttyUSB1
   COM10
*/

#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <SSD1306Wire.h>

#define DISPLAY_TITLE "Server v1.1.1.5"
#define MAX_CLIENTS 8
const char CONNECTED_SV = 'n';
const char RESERVE = 'v';
const char ADD = 'a';
const char CANCEL_ENTRY = 'c';
const char REMOVE = 'm';

const char* ssid     = "NodeMCU_AP";
const char* password = "pa$$word";

int PEOPLE_MAX = 0;

SSD1306Wire  display(0x3c, D2, D1);
WiFiServer _server(555);//Cria o objeto servidor na porta 555
WiFiClient _clients[MAX_CLIENTS];//Cria o objeto cliente.

void pinSetup();
void setupWifi();
void setupDisplay();
void peopleMaxSet();
void writeLineDisplay(int pixelBeginX, int line, String text);
void ledBlink();
void tcp();

int peopleCount = 0;
int gateCount[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
int reserved[4] = {0, 0, 0, 0};
int reserveds = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando...");
  setupPins();
  peopleMaxSet();
  setupWifi();
  setupDisplay();
}

void loop() {
  writeLineDisplay(0, 1, "Pes:" + String(peopleCount) + "/" + String(PEOPLE_MAX) + " Rsv:" + String(reserveds));
  for (int i = 0; i < 4; i++) {
    writeLineDisplay(0, i + 2, "pes: " + String(gateCount[i][0]) + " - std: " + String(gateCount[i][1]) + " - rsv: " + String(reserved[i]));
  }
  tcp();//Funçao que gerencia os pacotes e clientes TCP.
}

void setupWifi() {
  WiFi.mode(WIFI_AP);//Define o WiFi como Acess_Point.
  WiFi.softAP(ssid, password);//Cria a rede de Acess_Point.
  _server.begin();//Inicia o servidor TCP na porta declarada no começo.
}

void setupPins() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(14, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
}

void peopleMaxSet(){
  bool readPin1 = digitalRead(14);
  bool readPin2 = digitalRead(12);
 if(!readPin1 && !readPin2)
   PEOPLE_MAX = 1000;
 else if(readPin1 && readPin2)
   PEOPLE_MAX = 10;
 else
   PEOPLE_MAX = 100;
}

void setupDisplay() {
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, DISPLAY_TITLE);
  display.setFont(ArialMT_Plain_10);
  display.display();
}

void ledBlink() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(1);
  digitalWrite(LED_BUILTIN, HIGH);
}

void writeLineDisplay(int pixelBeginX, int line, String text) {
  display.setColor(BLACK);
  uint8_t pixelY = (line * 10) + (6 - line);
  display.fillRect(pixelBeginX, pixelY + 3, 128, 9); //coluna, linha, largura, altura
  display.setColor(WHITE);
  display.drawString(pixelBeginX, pixelY, text);
  display.display();
}

void tcp() {
  if (_server.hasClient()) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (!_clients[i] || !_clients[i].connected()) {
        if (_clients[i])
          _clients[i].stop();
        _clients[i] = _server.available();
//        Serial.print("New Client : ");
//        Serial.print(String(i + 1) + " - ");
        continue;
      }
    }
    WiFiClient _clients = _server.available();
    _clients.stop();
  }
  for (int i = 0; i < MAX_CLIENTS; i++) {
    //Detecta se há clientes conectados no servidor.
    if (_clients[i] && _clients[i].connected()) {
      ledBlink();
      //Verifica se o cliente conectado tem dados para serem lidos.
      if (_clients[i].available() > 0) {
        String msg_received = "";
        //Armazena cada Byte (letra/char) na String para formar a mensagem recebida.
        while (_clients[i].available() > 0) {
          char character = _clients[i].read();
          msg_received += character;
        }
        int gateId = msg_received.charAt(0) - '0';
        char gatePayload = msg_received.charAt(1);
        int total = peopleCount + reserveds;
        //Mostra a mensagem recebida do cliente no Serial Monitor.
        Serial.print("\n...Mensagem do cliente: |" + msg_received + "|");
        switch (gatePayload) {
          case CONNECTED_SV:
            _clients[i].println("Cliente conectado");
            break;
          case RESERVE:
            if ((total + 1) > 0 && (total + 1) <= PEOPLE_MAX) {
              reserved[gateId] = 1;
              _clients[i].println("release");
            }
            else {
              _clients[i].println("full");
            }
            break;
          case ADD:
            reserved[gateId] = 0;
            peopleCount++;
            gateCount[gateId][0]++;
            gateCount[gateId][1] = 1;
            _clients[i].println("added");
            break;
          case CANCEL_ENTRY:
            reserved[gateId] = 0;
            _clients[i].println("canceled");
            break;
          case REMOVE:
            if (peopleCount == 0)
              _clients[i].println("empty");
            else if (peopleCount > 0) {
              peopleCount--;
              //if (gateCount[gateId][0] > 0) {
              gateCount[gateId][0]--;
              //}
              gateCount[gateId][1] = -1;
              _clients[i].println("removed");
            }

            break;
          default:
            _clients[i].println("error");

        }
        for (int j = 0; j < MAX_CLIENTS; j++) {
          _clients[j].print("IP: ");
          _clients[j].println(_clients[i].remoteIP());
          _clients[j].print("Porta: ");
          _clients[j].println(_clients[i].remotePort());
          _clients[j].println(DISPLAY_TITLE);
          _clients[j].println("Pessoas:" + String(peopleCount) + "/" + String(PEOPLE_MAX) + " reservados:" + String(reserveds));
          for (int i = 0; i < 4; i++) {
            _clients[j].println("pessoas: " + String(gateCount[i][0]) + " - sentido: " + String(gateCount[i][1]) + " - reservado: " + String(reserved[i]));
          }

        }
        reserveds = reserved[0] + reserved[1] + reserved[2] + reserved[3];
      }
    }
  }
}
