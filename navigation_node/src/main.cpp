#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <QMC5883LCompass.h>
#include <math.h>

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
#define BAND 866E6 //NA Band

//OLED
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//IMU I2C for compass
//TwoWire Wire2 = TwoWire(1);
QMC5883LCompass compass;

//Variables
bool broadcast = false;
String data;
double rssi[3];
int dir;
int dev_id = 4;
int target_id = 0;
double distance[3];
double location0[2]; //Target device location
int location1[2] = {0,0};
int location2[2] = {10,0};
int location3[2] = {0,10};
double location4[2]; //Locating device location

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
double to_distance(double input_rssi, int one_m_power, int path_loss_constant){ //RSSI to convert, 1m Power, Path Loss Constant (Between 2 and 4)
  double output = pow(10,(one_m_power - input_rssi)/(10*path_loss_constant));
  return output;
}
double get_rssi(int node){
  broadcast=false;
  int val = 0;
  for(int i = 0; i < 3; i++){
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
            Serial.print(LoRa.packetRssi());
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
void trilaterate(int dist1, int dist2, int dist3){
  //trilateration code
  float x1 = location1[0];
  float y1 = location1[1];
  float x2 = location2[0];
  float y2 = location2[1];
  float x3 = location3[0];
  float y3 = location3[1];
  float r1 = dist1;
  float r2 = dist2;
  float r3 = dist3;
  float S = (pow(x3, 2.) - pow(x2, 2.) + pow(y3, 2.) - pow(y2, 2.) + pow(r2, 2.) - pow(r3, 2.)) / 2.0;
  float T = (pow(x1, 2.) - pow(x2, 2.) + pow(y1, 2.) - pow(y2, 2.) + pow(r2, 2.) - pow(r1, 2.)) / 2.0;
  float y = ((T * (x2 - x3)) - (S * (x2 - x1))) / (((y1 - y2) * (x2 - x3)) - ((y3 - y2) * (x2 - x1)));
  float x = ((y * (y1 - y2)) - T) / (x2 - x1);
  location0[0]=x;
  location0[1]=y;
}
String calculate_location(int rssi1, int rssi2, int rssi3){
  double dist1 = to_distance(rssi1,-68,2);
  double dist2 = to_distance(rssi2,-68,2);
  double dist3 = to_distance(rssi3,-68,2);
  String location = String(dist1) + "m," + String(dist2) + "m," + String(dist3) +"m nodes(1,2,3).";
  return location;
}
int request_location(int node){
  bool waiting=true;
  LoRa.beginPacket();
  LoRa.print(String(node));
  LoRa.endPacket();
  int to_return;
  while(waiting){
    int packetSize = LoRa.parsePacket();
    if (packetSize){
      if (LoRa.available()){
        data = LoRa.readString();
        if (data  == String(node)){
          waiting = false;
          to_return = data.toInt();
        }
        else{
          waiting = false;
          to_return = 0;
        }
      }
    }
  }
  return to_return;
}
void setup(){
  //Serial
  Serial.begin(9600);
  int test = -83;
  test = to_distance(test, -73, 2);
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
  //Initialise IMU
  compass.init();
  compass.setADDR(0x0D);
  compass.setCalibration(-1158, 737, -1612, 388, -1007, 851);

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
  //Wait for button press
  if(broadcast){
    //Get location of locating node
    broadcast = false;
    for(int node = 1; node < 4; node++){
      rssi[node-1] = get_rssi(node);
    }
    update_title("SOS");
    updateGUI(rssi[0],rssi[1],rssi[2]);
    distance[0]=to_distance(rssi[0], -68, 2);
    distance[1]=to_distance(rssi[1], -68, 2);
    distance[2]=to_distance(rssi[2], -68, 2);
    trilaterate(distance[0], distance[1], distance[2]);
    String location = calculate_location(rssi[0],rssi[1],rssi[2]);
    Serial.println("");
    Serial.println(location);
    Serial.println("x:" + String(location0[0]) + ",y:" + String(location0[1]));
    
    //Request location of target node
    bool waiting=true;
    LoRa.beginPacket();
    LoRa.print(target_id);
    LoRa.endPacket();     
    while(waiting){
      int packetSize = LoRa.parsePacket();
      if (packetSize){
        if (LoRa.available()){
          data = LoRa.readString();
          if (data  == String(dev_id)){
            waiting = false;
          }
        }
      }
    }
  }
  // else{
  //   display.clearDisplay();
  //   update_title("Search");
  //   //int location = request_location(0);
  //   while(broadcast){
  //     for(int node = 1; node < 4; node++){
  //     Serial.println("");
  //     Serial.print(node);
  //     Serial.print(",");
  //     broadcast=false;
  //     rssi[node] = get_rssi(node);
  //     }
  //   }
    
  //   // compass.read();
  //   // int a = compass.getAzimuth();
  //   // Serial.print(" Azimuth: ");
  //   // Serial.print(a);
  //   // updateGUI(0,0,a);
  //   // Serial.println(a);
  // }
}