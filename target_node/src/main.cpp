/*                           
=================================================
Radio Locator: Target Node
v1.0 - May 16 2022
Github: https://github.com/sam-hud/radio-locator

Released under the GNU General Public License v3
[https://www.gnu.org/licenses/gpl-3.0.en.html]
=================================================
*/

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Node ID - Set before programming device
int id = 0;

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

//Declare Variables
bool broadcast = false;
String data;
double rssi[3];

//Function to set the title of the OLED display
void update_title(String str){
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(str);
  display.display();
}
//Function to display the RSSI values on the OLED display
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
  display.clearDisplay();
  update_title("SOS"); //Set OLED title to notify status

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

//Main loop
void loop(){
  bool waiting = true; //Set device to wait for request message
  while(waiting){ //Wait for request message
    int received = LoRa.parsePacket(); //Check for request message
    if (received){ //If there is a response, continue
      if (LoRa.available()){
        data = LoRa.readString(); //Save the data to a string
        if (data  == String(id)){ //Check if the data matches this node ID
          broadcast = true; //Set the device to broadcast
          break; //Exit the while loop to allow broadcast to occur
        }
      }
    }
  }
  if(broadcast){
    broadcast = false; //Broadcast only once
    for(int node = 1; node < 4; node++){
      rssi[node-1] = get_rssi(node); //Get the mean RSSI to each beacon, save readings in an array
    }
    String msg = "4," + String(rssi[0]) + "," + String(rssi[1]) + "," + String(rssi[2]); //Create reply message from readings
    LoRa.beginPacket(); //Start communication
    LoRa.print(msg); //Send reply message
    LoRa.endPacket(); //Stop communication
    update_title("SOS"); //Set OLED title to notify status
    updateGUI(rssi[0],rssi[1],rssi[2]); //Display beacon readings on OLED display
  }
}