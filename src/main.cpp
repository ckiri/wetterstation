/* Die Wetterstation soll es ermöglichen Umgebungsmetriken aufzu-
 * nehmen und diese dann über einen MQTT Broker zu veröffentlichen.
 * Das System Sendet nur Daten, hört also keinem Topic zu.
 * 
 * Diese Daten können dann z.B. für Smarthomeanwendungen verwendet
 * werden.
 * 
 * Folgende Metriken werden aufgezeichnet:
 * Luftfeuchtigkeit, Luftqualität(Rauch) & Temperatur
 * 
 * Das System wird auf einem Espressif ESP8266 Microcontroller
 * realisiert.
 */

/* TODO: Auf dynamische Programmierung anpassen. D.h. keine delay()
 * Funktionen verwenden, sondern milis().
 */

#include <stdio.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>                                          // Abstraktionsschicht für Luftfeucht.- & Temp.-Sensor
#include <DHT.h>                                                      // Prog. Bibliothek für Temp.- & Luftfeucht.-Sensor
#include <DHT_U.h>              
#include <MQUnifiedsensor.h>                                          // Prog. Bibliothek für Luftqual.-Sensor
#include <ESP8266WiFi.h>
#include <PubSubClient.h>                                             // Prog. Bibliothek für MQTT

/* custom configuration */
#include "wifi_config.h"
#include "mqtt_config.h"

/* macros */
#define DHTPIN D3                                                     // Dateneingang Luftfeucht.- & Temp.-Sensor
#define DHTTYPE DHT11                                                 // Typ Luftfeucht.- & Temp.-Sensor
#define MQPIN A0                                                      // Analog Eingang "Rauchsensor"/MQ-2 Luftqual.-Sensor
#define CLEAN_AIR_FACTOR 9.83                                         // Aus Datenblatt entnommen
#define RL_VALUE 10                                                   // Lastwiderstand, Angabe in kOhm
#define BUZZPIN D2
#define BUZZFREQ 1000

/* global variables */
float smoke_curve[3] = {2.301,0.544,-0.497};                          // Berechn. Rauchkurve (Siehe Diagramm Datenblatt)
float ro = 0;                                                         // Widerstand des Sensors bei sauberer Luft

/* typedef */
sensors_event_t event;
uint32_t delay_ms;
sensor_t dht_sensor;

/* classes */
DHT_Unified dht(DHTPIN, DHTTYPE);
WiFiClient esp_client;
PubSubClient client(esp_client);

/* function declarations */
float mqCalibration();                                                // Kalibrierung Rauchsensor (ppm-Werte sind appoximiert)
float mqResistanceCalc(int);                                          // Berechnung der Widerstandswerte zum interpretieren der Einganssp.
int smokeLogScale(float, float *);                                    // Approximation des Rauchwertes
static void wifiSetup();
static void mqttSetup();
static void tempRead();                                               // Funktion zum Auslesen der Temperatur
static void humidRead();                                              // Funktion zum Auslesen der Luftfeuchtigkeit
static void mqRead();                                                 // Funktion zum Auslesen der "Rauchmenge"

/* setup, run once */
void setup() {
  Serial.begin(115200);                                               // Monitorgeschwindigkeit
  Serial.println("\n");
  Serial.println("******************");
  Serial.print("Wetterstation\n");
  Serial.println("******************");
  ro = mqCalibration();                                               // Kal. Werte zuweisen (Saubere Luft)
  //Serial.print("Ro = ");
  //Serial.print(ro);
  //Serial.print("kOhm\n");
  wifiSetup();
  mqttSetup();
  dht.begin();                                                        // Initialisieren Temp.- und Luftfeucht.-Sensor
  delay_ms = dht_sensor.min_delay / 1000;                             // Pause zw. Temp.- & Luftfeucht.-Messung 
}

float mqCalibration(){                                                // Kalibrierung des Rauch Sensors
  Serial.print("\n");
  Serial.print("\n");
  Serial.println("******************");
  Serial.print(F("Calibrating Smoke Sensor"));
  float val = 0;
  for(int i = 0; i < 20; i++) {
    val += mqResistanceCalc(analogRead(MQPIN));                       // Analogwert einlesen und an R-Berechnung übergeben
    Serial.print(F("."));
    delay(500);
  }
  val = val / 20;
  val = val / CLEAN_AIR_FACTOR;
  Serial.print("\n");
  Serial.println("Smoke Sensor Calibration is done!");
  Serial.println("******************");
  return val;
}

float mqResistanceCalc(int adc){
  return(((float)RL_VALUE * (1023 / adc) / adc));                     // Berechnung und Rückgabe des Widerstandes
}

void wifiSetup() {
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("\n");
  Serial.print("\n");
  Serial.println("******************");
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\n");
  Serial.print("Connected to the WiFi network!\n");
  Serial.println("******************");
}

void mqttSetup() {
  client.setServer(mqtt_server, mqtt_port);
  Serial.print("\n");
  Serial.print("\n");
  Serial.println("******************");
  Serial.print("Connecting to MQTT");
  while (!client.connected()) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.print("\n");
      Serial.println("Connected to MQTT Broker!");  
    } else {
      Serial.print("Failed with State: ");
      Serial.print(client.state());
      delay(2000);
    }
   Serial.println("******************"); 
  }
}

void tempRead(){
  dht.temperature().getEvent(&event);
  float temp_val = 0;
  char temp_str[3];
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);                                  // Sensor Wert lauschen
    Serial.println(F("°C"));                                          // Wert in seriellen Monitor schreiben
    temp_val = event.temperature;                                     
    sprintf(temp_str, "%f", temp_val);                                // Float in String convertieren
    const char* temp_cstr = temp_str;                                 // String für Übertragung in Const Char casten
    client.publish("/wetterstation/temperature", temp_cstr);          // Nachricht im Topic veröffentlichen
  }
}

void humidRead(){
  dht.humidity().getEvent(&event);
  float humid_val = 0;
  char humid_str[3]; 
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    humid_val = event.relative_humidity;
    sprintf(humid_str, "%f", humid_val);                              
    const char* humid_cstr = humid_str;
    client.publish("/wetterstation/humidity", humid_cstr);
  }
}

int smokeLogScale(float ratio, float* curve){                         // Berechnung des Rauchwertes in ppm
  return pow(10,(((log(ratio) - curve[1]) / curve[2]) + curve[0]));   // mit berechneten Werten der "Rauchkurve"
}

void mqRead(){
  int ppm = 0;
  char ppm_str[4];
  float rs = 0;
  for(int i = 0; i < 5; i++){ 
    rs += mqResistanceCalc(analogRead(MQPIN));                        // Sensorwert Messen und Berechnen
    delay(50);
  }
  rs = rs / 5;
  float rs_ro_ratio = rs / ro;
  ppm = smokeLogScale(rs_ro_ratio, smoke_curve);
  Serial.print(F("Smoke: "));
  Serial.print(ppm);
  Serial.println(F("ppm"));             
  sprintf(ppm_str, "%d", ppm);
  const char* ppm_cstr = ppm_str; 
  client.publish("/wetterstation/smoke", ppm_cstr);

  if(ppm > 100) {                                                     // BZZZZZ BZZZZZZ
    tone(BUZZPIN, BUZZFREQ);
  } else {
    noTone(BUZZPIN);
  }
}

/* continous loop */
void loop() {
  Serial.print("\n");
  Serial.print("\n");
  tempRead();
  delay(1000);
  humidRead();
  delay(1000);
  mqRead();
  delay(1000);
}
