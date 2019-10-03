#include <Arduino.h>
/*
   v1.1.1.4
   Cliente
   ESP32
   /dev/ttyUSB0
   /COM4
*/

#include <WiFi.h>
#define DEBOUNCE_INTERVAL 200 //intervalo em milissegundos em que o botão pode ser pressionado novamente
#define TICKET_GATE_TIMEOUT 5000 //tempo no qual a catraca fica liberada após ser aberta
#define GREEN 1
#define RED 0
#define DELAY_CONNECT 1000
#define DELAY_TEST_WIFI 200
#define DELAY_RECONNECT 5000

const char* CONNECTED_CL = "cn";
const char RESERVE = 'v';
const char ADD = 'a';
const char CANCEL_ENTRY = 'c';
const char REMOVE = 'm';

bool back2Online = false;

struct TicketGate {
  const uint8_t PIN_BTN_CHECK;
  const uint8_t PIN_BTN_TICKET_GATE;
  const uint8_t PIN_EXIT;
  const uint8_t PIN_LED_GREEN;
  const uint8_t PIN_LED_RED;
  bool checkState;
  bool ticketGateState;
  bool exitState;
  int timeout;
  bool state;
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

void setupPins();
void setupWifi();
bool sendPayload(String message);
void setLedColor(int ledId, bool ledState);
void opTicketGate(int tGateID);
void confirmTicketGate(int tGateID);
void timeoutTicketGate(int tGateID);
void exitTicketGate(int tGateID);
void ledBlink();
void changeAllLeds2Red();
bool testWifiConnection();
void wifiReconnect();
void systemOffline();

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");
  setupPins();
  changeAllLeds2Red();
  setupWifi();//Estabelece conexão com o AP
  if (testWifiConnection()) {
    sendPayload(CONNECTED_CL);//Envia a primeira mensagem para o servidor
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void loop() {
  while (!testWifiConnection()) {
    systemOffline();
    wifiReconnect();
    back2Online = true;
  }
  if (back2Online) {
    changeAllLeds2Red();
    back2Online = false;
  }
  bool buttonState;
  static unsigned long delayButtonCheck[4] = {0, 0, 0, 0};
  static unsigned long delayButtonTGate[4] = {0, 0, 0, 0};
  static unsigned long delayButtonExit[4] = {0, 0, 0, 0};

  for (int i = 0; i < 4; i++) {
    if ((millis() - delayButtonCheck[i]) > DEBOUNCE_INTERVAL) {
      buttonState = !digitalRead(tGate[i].PIN_BTN_CHECK);/*se o botão estiver pressionado, retorna LOW*/
      if (buttonState) {
        delayButtonCheck[i] = millis();
        if (buttonState != tGate[i].checkState) { //Se o botão de CHECK for pressionado, executar esse bloco
          if (tGate[i].state == 0)
            opTicketGate(i);
        }
        tGate[i].checkState = true;
      }
      tGate[i].checkState = buttonState;
    }
    if ((millis() - delayButtonTGate[i]) > DEBOUNCE_INTERVAL) {
      buttonState = !digitalRead(tGate[i].PIN_BTN_TICKET_GATE);/*se o botão estiver pressionado, retorna LOW*/
      if (buttonState) {
        delayButtonTGate[i] = millis();
        if (buttonState != tGate[i].ticketGateState) { //Se o botão de TICKET_GATE for pressionado, executar esse bloco
          if (tGate[i].state == 1)
            confirmTicketGate(i);
        }
        tGate[i].ticketGateState = true;
      }
      tGate[i].ticketGateState = buttonState;
    }
    if ((millis() - delayButtonExit[i]) > DEBOUNCE_INTERVAL) {
      buttonState = digitalRead(tGate[i].PIN_EXIT);/*se o botão estiver pressionado, retorna HIGH*/
      if (buttonState) {
        delayButtonExit[i] = millis();
        if (buttonState != tGate[i].exitState) { //Se o botão de EXIT for pressionado, executar esse bloco
          exitTicketGate(i);
        }
        tGate[i].exitState = true;
      }
      tGate[i].exitState = buttonState;
    }
    timeoutTicketGate(i);
  }
  delay(1);
}

void setupPins() {
  for (int i = 0; i < 4; i++) {
    pinMode(tGate[i].PIN_BTN_CHECK, INPUT);
    pinMode(tGate[i].PIN_BTN_TICKET_GATE, INPUT);
    pinMode(tGate[i].PIN_EXIT, INPUT);
    pinMode(tGate[i].PIN_LED_GREEN, OUTPUT);
    pinMode(tGate[i].PIN_LED_RED, OUTPUT);
  }
  pinMode(LED_BUILTIN, OUTPUT);
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wifi");
  unsigned long WifiTimeOut = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - WifiTimeOut < DELAY_CONNECT) {
    delay(500);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println(F("Wifi conectado"));
    Serial.println(F("endereço IP: "));
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println();
    Serial.println("Tempo de conexão WiFi excedido.");
    testWifiConnection();
  }
}

bool sendPayload(String message) {
  ledBlink();
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
  while (_client.available()) {
    char character = static_cast<char>(_client.read());
    serverResponse += character;
  }
  Serial.println(serverResponse);
  _client.stop();
  if (serverResponse == "release")
    return true;
  return false;
}

void setLedColor(int ledId, bool ledState) {
  tGate[ledId].state = ledState;
  digitalWrite(tGate[ledId].PIN_LED_GREEN, !ledState);
  digitalWrite(tGate[ledId].PIN_LED_RED, ledState);
}

void opTicketGate(int tGateID) {
  if (sendPayload(String(tGateID) + RESERVE)) {
    setLedColor(tGateID, GREEN);
    tGate[tGateID].timeout = millis();
  }
}

void timeoutTicketGate(int tGateID) {
  if (tGate[tGateID].state == 1) { //Se estiver aberto
    if ((millis() - tGate[tGateID].timeout) > TICKET_GATE_TIMEOUT) {
      setLedColor(tGateID, RED);
      sendPayload(String(tGateID) + String(CANCEL_ENTRY));//avisar o servidor que o portão não foi usado (adiciona 1 vaga livre)
    }
  }
}

void confirmTicketGate(int tGateID) {
  sendPayload(String(tGateID) + ADD);
  setLedColor(tGateID, RED);
}

void exitTicketGate(int tGateID) {
  sendPayload(String(tGateID) + REMOVE);//avisar o servidor que uma pessoa saiu (adiciona 1 vaga livre)
}

void ledBlink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1);
  digitalWrite(LED_BUILTIN, LOW);
}

void changeAllLeds2Red() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(tGate[i].PIN_LED_GREEN, HIGH);
    digitalWrite(tGate[i].PIN_LED_RED, LOW);
  }
}

bool testWifiConnection() {
  static unsigned long delayIsConnected = 0;
  static bool wifiConnected = false;
  if (millis() - delayIsConnected > DELAY_TEST_WIFI) {
    if (WiFi.status() == WL_CONNECTED)
      wifiConnected = true;
    else
      wifiConnected = false;
    delayIsConnected = millis();
  }
  return wifiConnected;
}

void wifiReconnect() {
  static unsigned long delayReconnect = 0;
  if (millis() - delayReconnect > DELAY_RECONNECT) {
    Serial.println(F("Tentando se reconectar ao wifi.."));
    WiFi.reconnect();
    delayReconnect = millis();
  }
}

void systemOffline() {
  static unsigned long delayBlink = 0;
  if (millis() - delayBlink > 500) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(tGate[i].PIN_LED_GREEN, HIGH);
      digitalWrite(tGate[i].PIN_LED_RED, !digitalRead(tGate[i].PIN_LED_RED));
    }
    delayBlink = millis();
  }
}
