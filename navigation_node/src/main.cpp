/*                           
=================================================
Radio Locator: Navigation Node
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
#include <QMC5883LCompass.h>
#include <math.h>

/*                Setup Variables              
===============================================
*/
//Node ID - Set before programming device
int id = 4;

//Target Node ID
int target_id = 0;

//System dependent variables (unique to each setup)
double location1[2] = {0,0}; //Location of beacon 1
double location2[2] = {10,76}; //Location of beacon 2
double location3[2] = {-125,150}; //Location of beacon 3
double target_1m[3] = {-63.519,-61.803,-51.038}; //Target node RSSI_1m constant for each beacon
double nav_1m[3] = {-67.535, -56.253, -60.992}; //Navigation node RSSI_1m constant for each beacon
double target_Cpl[3] = {2.9792,2.6987,2.6887}; //Target node path loss constant for each beacon
double nav_Cpl[3] = {2.4273,2.6467,2.7049}; //Navigation ndoe path loss constant for each beacon
int compass_offset = -80; //Compass calibration
/*=============================================*/      

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
double nav[3]; //Array for navigation node RSSI or distance (this node)
double target[3]; //Array for target node RSSI or distance
double location4[2] = {0,0}; //Location of the navgiation node (this device)
double location0[2] = {1,1}; //Target node location

//Declare compass functionality variables
int display_xc = 98;
int display_yc = 32;
double theta = 0;

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
            Serial.println(LoRa.packetRssi());
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
  float S = pow(r1,2)-pow(r2,2)-pow(x1,2)+pow(x2,2)-pow(y1,2)+pow(y2,2);
  float T = pow(r2,2)-pow(r3,2)-pow(x2,2)+pow(x3,2)-pow(y2,2)+pow(y3,2);
  float D = 4*(((x2-x1)*(y3-y2))-((x3-x2)*(y2-y1)));
  float x = ((2*S*(y3-y2))-(2*T*(y2-y1)))/D;
  float y = (2*T*(x2-x1)-2*S*(y2-y1))/D;
  if (node == target_id){ //Check which node location is being calculated, save to position vector for that node
  location0[0]=x;
  location0[1]=y;
  }
  else{
    location4[0]=x;
    location4[1]=y;
  }
}
//Function to split received data into usable values. Source:
//https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
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

  //Attach interrupt function to user button
  pinMode(BUTTON, INPUT);
  attachInterrupt(BUTTON,button_press,FALLING);

  //Initialise compass
  compass.init();
  compass.setADDR(0x0D);
  compass.setCalibration(-2095, 1818, -2473, 1875, -3866, 2143); //Calibrate compass

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
      nav[node-1] = get_rssi(node);
    }
    
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
          if(getValue(data,',',0) == String(id)){ //Check data to see if this node is addressed
            for(int i=0; i<3; i++){
              target[i]=getValue(data,',', i+1).toInt();
            }
            waiting = false; //End while loop since data is collected
          }
        }
      }
    }
    for(int i=0; i<3; i++){Serial.print(String(nav[i])+",");}
    Serial.println("");
    for(int i=0; i<3; i++){Serial.print(String(target[i])+",");}

    //Convert RSSI values to distances using known path loss constants
    for(int i=0; i<3; i++){nav[i]=to_distance(nav[i], nav_1m[i], nav_Cpl[i]);}; //Convert navigation RSSI to distances
    trilaterate(4, nav[0], nav[1], nav[2]); //Trilaterate position using calculated distances

    for(int i=0; i<3; i++){target[i] = to_distance(target[i],target_1m[i], target_Cpl[i]);} //Convert target RSSI to distances
    trilaterate(0, target[0], target[1], target[2]); //Trilaterate position using calculated distances

    //Calculate angle from North to navigate from navigation node (this node) to target node
    double a = location0[0] - location4[0];
    double b = location0[1] - location4[1];
    theta = atan(a/b)*(180/PI); //Direction from first quadrant in degrees

    //Ensure angles are in correct quadrants (following compass convention)
    if((location0[0]<location4[0]) && (location0[1]>location4[1])){theta+=360;} //Second quadrant
    else if ((location0[0]<location4[0]) && (location0[1]<location4[1])){theta+=180;} //Third quadrant
    else if((location0[0]>location4[0]) && (location0[1]<location4[1])){theta+=180;}//Fourth quadrant

    Serial.println("");
    Serial.println("NN | TN");
    Serial.println(String(location4[0]) + "," + String(location4[1])); //Print location to serial monitor
    Serial.println(String(location0[0]) + "," + String(location0[1])); //Print location to serial monitor
    Serial.println("Angle:" + String(theta)); //Print calculated angle
  }
    //Read compass bearing and save value to constant
    compass.read();
    int bearing = compass.getAzimuth();
    //Set compass calibration and make sure values stay in 360° range
    bearing+=compass_offset; 
    if(bearing<0){bearing+=360;}
    else if(bearing>360){bearing-=360;}

    double direction = bearing - theta + 180; //Find correct direction (+180 because OLED y axis is flipped)
    if(direction<0){direction+=360;} //Keep direction within 360° range
    else if(direction>360){direction-=360;}

    //OLED compass direction display calculations
    int compass_x = display_xc + 25*sin(direction*(PI/180)); //Calculate compass line x end point
    int compass_y = display_yc + 25*cos(direction*(PI/180)); //Calculate compass line y end point

    //Draw display
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.print(String(bearing));
    display.print(",");
    display.print(String(theta));
    display.setCursor (0,44);
    display.print(String(location4[0]) + "," + String(location4[1]));
    display.setCursor (0,54);
    display.print(String(location0[0]) + "," + String(location0[1]));
    display.drawLine(display_xc, display_yc, compass_x, compass_y, WHITE); //Direction line
    display.display();
    delay(100);
}