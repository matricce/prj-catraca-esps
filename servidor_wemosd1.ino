#include <Arduino.h>
/*
Servidor
*/
//v2.5.1.3c
#define CODE_LEVEL_DEBUG "versão 0.0.0.1"

#include <Wire.h>
#include <SSD1306Wire.h>

SSD1306Wire  display(0x3c, D2, D1);//remover
void writeLineDisplay(int pixelBeginX, int line, String text);
void setupDisplay();

void setup() {
  Serial.begin(115200);
  setupDisplay();//remover
}

void loop() {
  delay(1000);
  static int in = 0;
  static int ii = 1;
  writeLineDisplay(0, ii, String(in));
  Serial.println(ii);
  Serial.println(in);
  ii++;
  if(ii > 5)ii = 1;
  in++;
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
  display.drawString(0, 0, CODE_LEVEL_DEBUG);
  display.setFont(ArialMT_Plain_10);
  for(uint8_t i = 1; i<6; i++)
    writeLineDisplay(0, i, String(i));
  display.display();
}
