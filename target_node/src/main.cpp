#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Pins
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define BUTTON 0
#define COMPASS_SDA 23
#define COMPASS_SCL 22
#define COMPASS_DRDY 25

//LoRa Band
#define BAND 866E6 //UK Band

//OLED
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//Variables
bool broadcast = false;
String data;
double rssi[3];
int dev_id = 0;

void update_title(String str){
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(str);
  display.display();
}
void updateGUI(int rssi1,int rssi2,int rssi3){
  Serial.println("");
  Serial.print(rssi1);
  Serial.print(",");
  Serial.print(rssi2);
  Serial.print(",");
  Serial.print(rssi3);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10,20);
  display.print("1: ");
  display.print(rssi1);
  display.setCursor(10,30);
  display.print("2: ");
  display.print(rssi2);
  display.setCursor(10,40);
  display.print("3: ");
  display.print(rssi3);
  display.display();
}
void button_press(){
  broadcast = true;
}
double get_rssi(int node){
  broadcast=false;
  int val = 0;
  for(int i = 0; i < 10; i++){
    bool waiting=true;
    LoRa.beginPacket();
    LoRa.print(String(node));
    LoRa.endPacket();
    while(waiting){
      int packetSize = LoRa.parsePacket();
      if (packetSize){
        if (LoRa.available()){
          data = LoRa.readString();
          if (data  == String(node)){
            val += LoRa.packetRssi();
            Serial.print(String(LoRa.packetRssi()) + ",");
            waiting = false;
          }
          else{
            continue;
          }
        } 
      }
    }
  }
  return val/3;
}
void setup(){
  //Serial
  Serial.begin(9600);
  //Reset OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //Initialise OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {
    Serial.println(F("OLED not found"));
    while(1);
  }

  //Button
  pinMode(BUTTON, INPUT);
  attachInterrupt(BUTTON,button_press,FALLING);

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
  if (packetSize){
    if (LoRa.available()){
      data = LoRa.readString();
      if (data  == String(dev_id)){
        broadcast = true;
      }
    }
  }
  if(broadcast){
    broadcast = false;
    for(int node = 1; node < 4; node++){
      Serial.println(String(node) + ": ");
      rssi[node-1] = get_rssi(node);
    }
    // String msg = "4," + String(rssi[0]) + "," + String(rssi[1]) + "," + String(rssi[2]);
    // LoRa.beginPacket();
    // LoRa.print(msg);
    // LoRa.endPacket();
    // update_title("SOS");
    // updateGUI(rssi[0],rssi[1],rssi[2]);
  }
}