#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <QMC5883LCompass.h>
#include <math.h>

/*
Navigation Node Code
12/5/22
By Sam Hudson
*/


//Node ID - Set before programming device
int id = 4;

//Target Node ID
int target_id = 0;

//Set Pins
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

//OLED Display Pins and Setup
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//Create compass object
QMC5883LCompass compass;

//Declare Variables
bool broadcast = false;
String data;
double rssi[3];
int dir;
double distance[3];
double location0[2]; //Target node location
int location1[2] = {0,0}; //Location of beacon 1
int location2[2] = {10,0}; //Location of beacon 2
int location3[2] = {0,10}; //Location of beacon 3
double location4[2]; //Location of the navgiation node (this device)

//Function to set the title of the OLED display
void update_title(String str){
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(str);
  display.display();
}

//Function for button press interrupt - set device to broadcast
void button_press(){
  broadcast = true;
}

//Function to convert RSSI to distance
double to_distance(double input_rssi, int one_m_power, int path_loss_constant){ //RSSI to convert, 1m Power, Path Loss Constant
  double output = pow(10,(one_m_power - input_rssi)/(10*path_loss_constant));
  return output;
}

//Function to get the mean RSSI from one beacon
double get_rssi(int node){ //Takes the beacon ID as the input
  broadcast=false;
  int val = 0; //For storing the total of the RSSI values to calculate the mean
  int count = 0; //For mean calculation, in case fewer than 3 values are used
  for(int i = 0; i < 3; i++){ //Take 3 RSSI readings
    bool waiting=true; //At the start of each loop set the device to wait for a reply
    LoRa.beginPacket(); //Start communication
    LoRa.print(String(node)); //Send the ID of the beacon
    LoRa.endPacket(); //Stop communication
    while(waiting){ //Wait for response
      int packetSize = LoRa.parsePacket(); //Check for response
      if (packetSize){ //If there is a response, continue
        if (LoRa.available()){
          data = LoRa.readString(); //Save data to string
          if (data  == String(node)){ //Check if data matches the beacon ID
            count++; //Increase count for every successful reply received
            val += LoRa.packetRssi(); //Add the reading to the RSSI sum
            waiting = false; //Break while loop, continue to next reading
          }
          else{
            continue; //If a reply is received that does not match the beacon ID, skip.
          }
        } 
      }
    }
  }
  return val/count; //Return the mean RSSI value
}

//Function to trilaterate the 3 distances into the node x,y position
void trilaterate(int node, int dist1, int dist2, int dist3){
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
  if (node == target_id){ //Check which node location is being calculated, save to position vector for that node
  location0[0]=x;
  location0[1]=y;
  }
  else{
    location4[0]=x;
    location4[1]=y;
  }
}

//Device setup
void setup(){
  //Start serial monitor for debugging
  Serial.begin(9600);

  //Reset OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //Initialise OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { //Set to 3.3V mode
    Serial.println(F("OLED did not start")); //Print to serial if OLED fails to start
    while(1); //Stop device
  }

  //Attach interrupt function to user button
  pinMode(BUTTON, INPUT);
  attachInterrupt(BUTTON,button_press,FALLING);

  //Initialise compass
  compass.init();
  compass.setADDR(0x0D);
  compass.setCalibration(-1158, 737, -1612, 388, -1007, 851); //Calibrate compass

  //Attach LoRa pins to device SPI
  SPI.begin(SCK, MISO, MOSI, SS);

  //Set LoRa pins
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) { //Start LoRa
    Serial.println("LoRa did not start"); //Print to serial if LoRa fails to start
    while (1); //Stop device
  }
  Serial.println("LoRa started successfully"); //Print to serial if LoRa started successfully
}

void loop(){
  //Wait for button press
  if(broadcast){
    //Get location of navigation node (this device)
    broadcast = false;
    for(int node = 1; node < 4; node++){
      rssi[node-1] = get_rssi(node);
    }
    //Convert RSSI values to distances using known path loss constants
    distance[0]=to_distance(rssi[0], -68, 2);
    distance[1]=to_distance(rssi[1], -68, 2);
    distance[2]=to_distance(rssi[2], -68, 2);
    trilaterate(4, distance[0], distance[1], distance[2]); //Trilaterate position using calculated distances
    Serial.println("x:" + String(location4[0]) + ",y:" + String(location4[1])); //Print location to serial monitor
    
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
          if (data  == String(id)){
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