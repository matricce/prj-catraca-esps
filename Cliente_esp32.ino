#include <Arduino.h>
/*
Cliente
*/

/*
 * CHECK é o botão que simula o gatilho para checar se a catraca pode ou não ser liberada
 * TICKET_GATE simula a catraca sendo girada para dentro (uma pessoa entrou)
 * EXIT simula a catraca sendo girada para fora (uma pessoa saiu)
 */

#define DEBOUNCE_INTERVAL 200 /*intervalo em milissegundos em que o botão pode ser pressionado novamente*/
struct TicketGate{
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
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");
  
  for(int i = 0; i<4; i++){
    pinMode(tGate[i].PIN_BTN_CHECK, INPUT);
    pinMode(tGate[i].PIN_BTN_TICKET_GATE, INPUT);
    pinMode(tGate[i].PIN_EXIT, INPUT);
    pinMode(tGate[i].PIN_LED_GREEN, OUTPUT);
    pinMode(tGate[i].PIN_LED_RED, OUTPUT);
  }
}

void loop() {
  bool buttonState;
  static unsigned long delayButtonCheck[4] = {0, 0, 0, 0};
  static unsigned long delayButtonTGate[4] = {0, 0, 0, 0};
  static unsigned long delayButtonExit[4] = {0, 0, 0, 0};
  for(int i = 0; i<4; i++){
    if((millis() - delayButtonCheck[i]) > DEBOUNCE_INTERVAL){
      buttonState = !digitalRead(tGate[i].PIN_BTN_CHECK);/*se o botão estiver pressionado, retorna LOW*/
      if(buttonState){
        delayButtonCheck[i] = millis();
        if(buttonState != tGate[i].checkStatus){//Se o botão de CHECK for pressionado, executar esse bloco
          Serial.print(F("botão CHECK-"));
          Serial.print(i);
          Serial.println(F(" pressionado"));
        }
        tGate[i].checkStatus = true;
      }
      tGate[i].checkStatus = buttonState;
    }
    if((millis() - delayButtonTGate[i]) > DEBOUNCE_INTERVAL){
      buttonState = !digitalRead(tGate[i].PIN_BTN_TICKET_GATE);/*se o botão estiver pressionado, retorna LOW*/
      if(buttonState){
        delayButtonTGate[i] = millis();
        if(buttonState != tGate[i].ticketGateStatus){//Se o botão de TICKET_GATE for pressionado, executar esse bloco
          Serial.print(F("botão TICKET_GATE-"));
          Serial.print(i);
          Serial.println(F(" pressionado"));
        }
        tGate[i].ticketGateStatus = true;
      }
      tGate[i].ticketGateStatus = buttonState;
    }
    if((millis() - delayButtonExit[i]) > DEBOUNCE_INTERVAL){
      buttonState = digitalRead(tGate[i].PIN_EXIT);/*se o botão estiver pressionado, retorna HIGH*/
      if(buttonState){
        delayButtonExit[i] = millis();
        if(buttonState != tGate[i].exitStatus){//Se o botão de EXIT for pressionado, executar esse bloco
          Serial.print(F("botão EXIT-"));
          Serial.print(i);
          Serial.println(F(" pressionado"));
        }
        tGate[i].exitStatus = true;
      }
      tGate[i].exitStatus = buttonState;
    }
//    digitalWrite(tGate[i].PIN_LED_GREEN, !digitalRead(tGate[i].PIN_LED_GREEN));
//    digitalWrite(tGate[i].PIN_LED_RED, !digitalRead(tGate[i].PIN_LED_GREEN));
  }
}
