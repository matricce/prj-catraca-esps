/*
   Cliente
   ESP32
   /dev/ttyUSB0
   /COM4
*/

/*
   CHECK é o botão que simula o gatilho para checar se a catraca pode ou não ser liberada
   TICKET_GATE simula a catraca sendo girada para dentro (uma pessoa entrou)
   EXIT simula a catraca sendo girada para fora (uma pessoa saiu)
*/
#include <WiFi.h>
#define DEBOUNCE_INTERVAL 200 /*intervalo em milissegundos em que o botão pode ser pressionado novamente*/
struct TicketGate {
  const uint8_t PIN_BTN_CHECK;
  const uint8_t PIN_BTN_TICKET_GATE;
  const uint8_t PIN_EXIT;
  const uint8_t PIN_LED_GREEN;
  const uint8_t PIN_LED_RED;
  bool checkStatus;
  bool ticketGateStatus;
  bool exitStatus;
};

TicketGate tGate[] = {
  {15, 22, 35, 4, 16},
  {13, 21, 34, 14, 27},
  {26, 25, 39, 33, 32},
  {17, 5, 36, 18, 19}
};

const char* ssid     = "NodeMCU_AP";
const char* password = "pa$$word";

const char* host = "192.168.4.1";
const uint16_t port = 555;

void pinsSetup();
void wifiSetup();
bool sendPayload(String message);

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");
  pinsSetup();
  wifiSetup();//Estabelece conexão com o AP
  sendPayload("Comunicação iniciada");//Envia a primeira mensagem para o servidor
}

void loop() {
  bool buttonState;
  static unsigned long delayButtonCheck[4] = {0, 0, 0, 0};
  static unsigned long delayButtonTGate[4] = {0, 0, 0, 0};
  static unsigned long delayButtonExit[4] = {0, 0, 0, 0};
  for (int i = 0; i < 4; i++) {
    if ((millis() - delayButtonCheck[i]) > DEBOUNCE_INTERVAL) {
      buttonState = !digitalRead(tGate[i].PIN_BTN_CHECK);/*se o botão estiver pressionado, retorna LOW*/
      if (buttonState) {
        delayButtonCheck[i] = millis();
        if (buttonState != tGate[i].checkStatus) { //Se o botão de CHECK for pressionado, executar esse bloco
//          Serial.print(F("botão CHECK-"));
//          Serial.print(i);
//          Serial.println(F(" pressionado"));
          sendPayload("botão CHECK-" + String(i) + " pressionado");
        }
        tGate[i].checkStatus = true;
      }
      tGate[i].checkStatus = buttonState;
    }
    if ((millis() - delayButtonTGate[i]) > DEBOUNCE_INTERVAL) {
      buttonState = !digitalRead(tGate[i].PIN_BTN_TICKET_GATE);/*se o botão estiver pressionado, retorna LOW*/
      if (buttonState) {
        delayButtonTGate[i] = millis();
        if (buttonState != tGate[i].ticketGateStatus) { //Se o botão de TICKET_GATE for pressionado, executar esse bloco
//          Serial.print(F("botão TICKET_GATE-"));
//          Serial.print(i);
//          Serial.println(F(" pressionado"));
          sendPayload("botão TICKET_GATE-" + String(i) + " pressionado");
        }
        tGate[i].ticketGateStatus = true;
      }
      tGate[i].ticketGateStatus = buttonState;
    }
    if ((millis() - delayButtonExit[i]) > DEBOUNCE_INTERVAL) {
      buttonState = digitalRead(tGate[i].PIN_EXIT);/*se o botão estiver pressionado, retorna HIGH*/
      if (buttonState) {
        delayButtonExit[i] = millis();
        if (buttonState != tGate[i].exitStatus) { //Se o botão de EXIT for pressionado, executar esse bloco
//          Serial.print(F("botão EXIT-"));
//          Serial.print(i);
//          Serial.println(F(" pressionado"));
          sendPayload("botão EXIT-" + String(i) + " pressionado");
        }
        tGate[i].exitStatus = true;
      }
      tGate[i].exitStatus = buttonState;
    }
    //    digitalWrite(tGate[i].PIN_LED_GREEN, !digitalRead(tGate[i].PIN_LED_GREEN));
    //    digitalWrite(tGate[i].PIN_LED_RED, !digitalRead(tGate[i].PIN_LED_GREEN));
  }
}

void pinsSetup() {
  for (int i = 0; i < 4; i++) {
    pinMode(tGate[i].PIN_BTN_CHECK, INPUT);
    pinMode(tGate[i].PIN_BTN_TICKET_GATE, INPUT);
    pinMode(tGate[i].PIN_EXIT, INPUT);
    pinMode(tGate[i].PIN_LED_GREEN, OUTPUT);
    pinMode(tGate[i].PIN_LED_RED, OUTPUT);
  }
}

void wifiSetup() {
  Serial.print("Conectando ao ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);//Configura para "station mode" assim não cria uma rede AP, interferindo na comunicação
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("WiFi conectado"));
  Serial.println(F("endereço IP: "));
  Serial.println(WiFi.localIP());
}

bool sendPayload(String message) {
  String serverResponse = "";
  if (message == "")
    return false;
  // A classe WiFiClient é utilizada para criar conexões TCP
  WiFiClient _client;
  if (!_client.connect(host, port)) {
    Serial.println(F("<< falha na conexão"));
    delay(5000);
    return false;
  }
  //A mensagem é enviada para o servidor caso haja conexão
  if (_client.connected()) {
    _client.print(message);//Somente 1 mensagem por conexão
  }
  //aguarda os dados ficarem disponíveis
  unsigned long timeout = millis();
  while (_client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(F(">>> Tempo de conexão excedido!"));
      _client.stop();
      delay(3000);
      return false;
    }
  }
  //Lê todas as linhas da resposta do servidor
//  Serial.println(F("<< Recebendo do servidor remoto: "));
  while (_client.available()) {
    char character = static_cast<char>(_client.read());
    serverResponse += character; 
  }
  Serial.println(serverResponse);
  // Close the connection
//  Serial.println("\n<< encerrando conexão");
  _client.stop();
  if(serverResponse != "")
    return true;
  return false;
}
