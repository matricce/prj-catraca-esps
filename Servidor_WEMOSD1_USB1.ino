/*
   Servidor
   WEMOS D1
   /dev/ttyUSB1
   COM10
*/

#include <ESP8266WiFi.h>
#include <WiFiServer.h> 
#include <SSD1306Wire.h>

#define DISPLAY_TITLE "Server v0.0.0.1"

const char* ssid     = "NodeMCU_AP";
const char* password = "pa$$word";

SSD1306Wire  display(0x3c, D2, D1);
WiFiServer _server(555);//Cria o objeto servidor na porta 555
WiFiClient _client;//Cria o objeto cliente.

void tcp();
void wifiSetup();
void writeLineDisplay(int pixelBeginX, int line, String text);
void setupDisplay();

void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando...");
  wifiSetup();
  setupDisplay();
}

uint32_t verificador = 0;

void loop() {
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
      //Mostra a mensagem recebida do cliente no Serial Monitor.
      Serial.print("\n...Mensagem do cliente: " + msg_received + "");
      //Envia uma resposta para o cliente
      _client.println(">> O servidor recebeu sua mensagem: " + msg_received);
      writeLineDisplay(0, 1, msg_received);
      
    }
  }
  //Se nao houver cliente conectado,
  else {
    _client = _server.available();//Disponabiliza o servidor para o cliente se conectar.
    delay(1);
  }
}

void writeLineDisplay(int pixelBeginX, int line, String text){
  display.setColor(BLACK);
  uint8_t pixelY = (line * 10) + (6-line);
  display.fillRect(pixelBeginX, pixelY+3, 128, 9);//coluna, linha, largura, altura
  display.setColor(WHITE);
  display.drawString(pixelBeginX, pixelY, text);
  display.display();
}

void setupDisplay(){
  display.init();
  display.flipScreenVertically();
  //Apaga o display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  //Seleciona a fonte
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, DISPLAY_TITLE);
  display.setFont(ArialMT_Plain_10);
  for(uint8_t i = 1; i<6; i++)
    writeLineDisplay(0, i, String(i));
  display.display();
}
