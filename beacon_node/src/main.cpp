#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>

//STATIC NODE CODE

//Pins
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define START_CLOCK 21
#define STOP_CLOCK 12
#define RXD 13
#define TXD 17

//LoRa Band
#define BAND 866E6 //NA band

void setup(){
  // //Serial
  // Serial.begin(9600);

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //Set LoRa pins
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("LoRa failed");
    while (1);
  }
  Serial.println("LoRa Initialised");
}

void loop(){
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    if (LoRa.available()) {
      if (LoRa.readString() == "3"){
        String data = "3"; //+ String(LoRa.packetRssi());
        LoRa.beginPacket();
        LoRa.print(data);
        LoRa.endPacket();
      }
    }
  }
}