// Librerias
#include <ESP8266WiFi.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <SPI.h>

// Conección al Servidor
const char *ssid = "SEMARD";
const char *password = "SEMARD123";
const char *mqtt_server = "192.168.0.200";

// Constantes del RFID al WIFI
constexpr uint8_t RST_PIN = 2;       // Pin Reset
constexpr uint8_t SS_1_PIN = 15;     // Pin de señal Mdulo 1
constexpr uint8_t SS_2_PIN = 16;     // Pin de señal Mdulo 2
constexpr uint8_t NR_OF_READERS = 2; // Pin de Reset

byte ssPins[] = {SS_1_PIN, SS_2_PIN};

// Crear la Instancia MFRC522
MFRC522 rfid[NR_OF_READERS];

/*********************************************************/
/**********************  FUNCIONES DE RFID *************************/

String dump_byte_array(byte *buffer, byte bufferSize);
void checkFirwareVersion();
void readDataRFID();

/*********************************************************/

/*********************************************************/
/********************** FUNCIONES DE WIFI *************************/

void setup_wifi();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void mqttFunction();
void json();

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Inicialización

void setup() {

  Serial.begin(115200);
  SPI.begin();
  checkFirwareVersion();

  pinMode(BUILTIN_LED, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  readDataRFID();
  mqttFunction();
}

// Tamaño del Buffer de Mensaje
String dump_byte_array(byte *buffer, byte bufferSize) {
  String mensaje = "";
  for (byte i = 0; i < bufferSize; i++) {
    String c = buffer[i] < 0x10 ? " 0" : " ";
    mensaje = mensaje + c;
    Serial.print(buffer[i], HEX);
  }
  Serial.println(mensaje);
  return mensaje;
}

// Verificacion de Lectura
void checkFirwareVersion() {
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    rfid[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("\nLectura "));
    Serial.print(reader);
    Serial.print(F(": "));
    rfid[reader].PCD_DumpVersionToSerial();
  }
}

// Lectura de la tarjeta RFID
void readDataRFID() {
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {

    // Busca Nuevas tarjetas

    if (rfid[reader].PICC_IsNewCardPresent() &&
        rfid[reader].PICC_ReadCardSerial()) {
      Serial.print(F("/nLectura Principal "));
      Serial.print(reader);

      Serial.print(F("/nID Unico de Tarjeta:"));
      String publishing =
          dump_byte_array(rfid[reader].uid.uidByte, rfid[reader].uid.size);
      const char *payload = publishing.c_str();
      client.publish("outTopic", payload);
      Serial.println();
      Serial.print(F("Tipo de Tarjeta: "));
      MFRC522::PICC_Type piccType =
          rfid[reader].PICC_GetType(rfid[reader].uid.sak);
      Serial.println(rfid[reader].PICC_GetTypeName(piccType));

      // Halt PICC
      rfid[reader].PICC_HaltA();

      // Para la Encriptación de lectura

      rfid[reader].PCD_StopCrypto1();
    }
    // if (mfrc522[reader].PICC_IsNewC
  }
  // for(uint8_t reader
}

// Funcion de coneccion al WIFI
void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Conectado a:  ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conexion a WIFI: ");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Mensaje Recivido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);

  } else {
    digitalWrite(BUILTIN_LED, HIGH);
  }
}

// Funcion de Reconexion
void reconnect() {

  while (!client.connected()) {
    Serial.print("Intentando Conexion al MQTT...");

    if (client.connect("Cliente ESP8266")) {
      Serial.println("Conectado");

      client.publish("outTopic", "Rafa, Si Sirve");

      client.subscribe("inTopic");
    } else {
      Serial.print("Fallo, RC = ");
      Serial.print(client.state());
      Serial.println(" Reconectando en 5 segundos...");

      delay(5000);
    }
  }
}

// Funcion MQTT
void mqttFunction() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {

    // Busca Nuevas tarjetas

    if (rfid[reader].PICC_IsNewCardPresent() &&
        rfid[reader].PICC_ReadCardSerial()) {
      Serial.print(F("\nLectura Principal "));
      Serial.print(reader);

      Serial.print(F("\nID Unico de Tarjeta:"));
      String publishing =
          dump_byte_array(rfid[reader].uid.uidByte, rfid[reader].uid.size);
      const char *payload = publishing.c_str();
      client.publish("outTopic", payload);
      Serial.println();
      Serial.print(F("Tipo de Tarjeta: "));
      MFRC522::PICC_Type piccType =
          rfid[reader].PICC_GetType(rfid[reader].uid.sak);
      Serial.println(rfid[reader].PICC_GetTypeName(piccType));

      // Halt PICC
      rfid[reader].PICC_HaltA();

      // Para la Encriptación de lectura

      rfid[reader].PCD_StopCrypto1();
    }
    // if (mfrc522[reader].PICC_IsNewC
  }

  // long now = millis();
  // if (now - lastMsg > 2000) {
  //   lastMsg = now;
  //   ++value;
  //   snprintf(msg, 75, "Mensaje #%ld", value);
  //   Serial.print("Mensaje Publicado: ");
  //   Serial.println(msg);
  //   client.publish("outTopic", msg);
  // }
}

// Función json
void json() {

  StaticJsonBuffer<200> jsonBuffer;

  JsonObject &root = jsonBuffer.createObject();
  root["sensor"] = "gps";
  root["time"] = 1351824120;

  JsonArray &data = root.createNestedArray("data");
  data.add(48.756080);
  data.add(2.302038);

  root.printTo(Serial);
}
