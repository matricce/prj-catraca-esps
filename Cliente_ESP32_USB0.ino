/*
 * Cliente
 * ESP32
 * /dev/ttyUSB0
 * /COM4
 */
 
#include <WiFi.h>

const char* ssid     = "NodeMCU_AP";
const char* password = "pa$$word";

const char* host = "192.168.4.1";
const uint16_t port = 555;

void wifiSetup(){
  Serial.print("Conectando ao ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);//Configura para "station mode" assim não cria uma rede AP, interferindo na comunicação
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado");
  Serial.println("endereço IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  wifiSetup();//Estabelece conexão com o AP
}

void loop() {
  Serial.print("<< se conectando com ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("<< falha na conexão");
    delay(5000);
    return;
  }

  // This will send a string to the server
  Serial.println("<< enviando dados para o servidor");
  if (client.connected()) {
    client.print("<< olá de ESP32");
    client.print("<< estou online");
  }

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(6000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("<< recebendo do servidor remoto");
  // not testing 'client.connected()' since we do not need to send data here
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }

  // Close the connection
  Serial.println("____");
  Serial.println("<< encerrando conexão");
  client.stop();

  delay(6000); // execute once every 5 minutes, don't flood remote service
}
