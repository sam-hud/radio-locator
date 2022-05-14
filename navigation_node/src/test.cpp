// #include <Arduino.h>
// #include <SPI.h>
// #include <LoRa.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #include <QMC5883LCompass.h>

// //Pins
// #define SCK 5
// #define MISO 19
// #define MOSI 27
// #define SS 18
// #define RST 14
// #define DIO0 26
// #define BUTTON 0
// #define IMU_SDA 23
// #define IMU_SCL 22
// #define IMU_DRDY 25

// //LoRa Band
// #define BAND 915E6 //NA Band

// //OLED
// #define OLED_SDA 4
// #define OLED_SCL 15 
// #define OLED_RST 16
// #define SCREEN_WIDTH 128 // OLED display width, in pixels
// #define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// //IMU I2C for compass
// TwoWire IMU = TwoWire(1);
// QMC5883LCompass compass;

// //Variables
// bool waiting = true;
// bool broadcast = false;
// String data;
// int rssi[10];
// int rssi1 = 0;
// int rssi2 = 0;
// int rssi3 = 0;
// int dir;

// void updateGUI(){
//     Serial.println("");
//     Serial.print(rssi1);
//     Serial.print(",");
//     Serial.print(rssi2);
//     Serial.print(",");
//     Serial.print(rssi3);
//     display.clearDisplay();
//     display.setTextSize(2);
//     display.setTextColor(WHITE);
//     display.setCursor(0, 0);
//     display.println("TN(0)");
//     display.setTextSize(1);
//     display.setCursor(10,20);
//     display.print("1: ");
//     display.print(rssi1);
//     display.setCursor(10,30);
//     display.print("2: ");
//     display.print(rssi2);
//     display.setCursor(10,40);
//     display.print("3: ");
//     display.print(rssi3);
//     display.display();
// }
// void button_press(){
//   broadcast = true;
// }
// void setup(){
//   //Serial
//   Serial.begin(9600);

//   //Reset OLED
//   pinMode(OLED_RST, OUTPUT);
//   digitalWrite(OLED_RST, LOW);
//   delay(20);
//   digitalWrite(OLED_RST, HIGH);

//   //Initialise OLED
//   //Wire.begin(OLED_SDA, OLED_SCL);
//   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {
//     Serial.println(F("OLED not found"));
//     while(1);
//   }
//   updateGUI();
//   //Initialise IMU
//   Wire.begin(IMU_SDA, IMU_SCL);
//   compass.setADDR(0x0D);
//   compass.init();
//  compass.setCalibration(-1158, 737, -1612, 388, -1007, 851);
//   pinMode(25,INPUT);

//   //Button
//   pinMode(BUTTON, INPUT);
//   attachInterrupt(BUTTON,button_press,FALLING);

//   //SPI LoRa pins
//   SPI.begin(SCK, MISO, MOSI, SS);
//   //Set LoRa pins
//   LoRa.setPins(SS, RST, DIO0);

//   if (!LoRa.begin(BAND)) {
//     Serial.println("LoRa failed");
//     while (1);
//   }
//   Serial.println("LoRa Initialised");
//   delay(100);
// }
// void loop(){
// int x, y, z, a, b;
// 	char myArray[3];
	
// 	compass.read();
  
// 	x = compass.getX();
// 	y = compass.getY();
// 	z = compass.getZ();
	
// 	a = compass.getAzimuth();
	
// 	b = compass.getBearing(a);

// 	compass.getDirection(myArray, a);
  
  
// 	Serial.print("X: ");
// 	Serial.print(x);

// 	Serial.print(" Y: ");
// 	Serial.print(y);

// 	Serial.print(" Z: ");
// 	Serial.print(z);

// 	Serial.print(" Azimuth: ");
// 	Serial.print(a);

// 	Serial.print(" Bearing: ");
// 	Serial.print(b);

// 	Serial.print(" Direction: ");
// 	Serial.print(myArray[0]);
// 	Serial.print(myArray[1]);
// 	Serial.print(myArray[2]);

// 	Serial.println();

// 	delay(250);
//   if(broadcast){
//     for(int node = 1; node < 4; node++){
//       Serial.println("");
//       Serial.print(node);
//       Serial.print(",");
//       broadcast=false;
//       for(int i = 0; i < 2; i++){
//         waiting=true;
//         LoRa.beginPacket();
//         LoRa.print(String(node));
//         LoRa.endPacket();
//         while(waiting){
//           int packetSize = LoRa.parsePacket();
//           if (packetSize) {
//             if (LoRa.available()) {
//               data = LoRa.readString();
//               if (data  == String(node)){
//                 // rssi[i] = LoRa.packetRssi();
//                 // rssi[4+i] = (data.substring(1, data.length())).toInt();
//                 // Serial.print(rssi[i]);
//                 // Serial.print(",");
//                 // Serial.print(rssi[4+i]);
//                 // Serial.print(",");
//                 Serial.println(LoRa.packetRssi());
//                 waiting = false;
//               }
//               else{
//                 continue;
//               }
//             } 
//           }
//         }
//       }
//       // if(node==1){tx_1 = tx_time;}
//       // else if(node==2){tx_2 = tx_time;}
//       // else if(node==3){tx_3 = tx_time;}
//     }
//     // tx_1 = ((tx_1-calib_1)/(240E6))*299702547*0.5;
//     // tx_2 = ((tx_2-calib_2)/(240E6))*299702547*0.5;
//     // tx_3 = ((tx_3-calib_3)/(240E6))*299702547*0.5;
//     updateGUI();
//   }
// }


// #include <Arduino.h>
// #include <SPI.h>
// #include <LoRa.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #include <QMC5883LCompass.h>
// #include <math.h>

// //Pins
// #define SCK 5
// #define MISO 19
// #define MOSI 27
// #define SS 18
// #define RST 14
// #define DIO0 26
// #define BUTTON 0
// #define COMPASS_SDA 23
// #define COMPASS_SCL 22
// #define COMPASS_DRDY 25

// //LoRa Band
// #define BAND 915E6 //NA Band

// //OLED
// #define OLED_SDA 4
// #define OLED_SCL 15 
// #define OLED_RST 16
// #define SCREEN_WIDTH 128 // OLED display width, in pixels
// #define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// //IMU I2C for compass
// TwoWire Wire2 = TwoWire(1);
// QMC5883LCompass compass;

// //Variables
// bool waiting = true;
// bool broadcast = false;
// String data;
// int rssi[10];
// int rssi1 = 0;
// int rssi2 = 0;
// int rssi3 = 0;
// int dir;

// void updateGUI(){
//     Serial.println("");
//     Serial.print(rssi1);
//     Serial.print(",");
//     Serial.print(rssi2);
//     Serial.print(",");
//     Serial.print(rssi3);
//     display.clearDisplay();
//     display.setTextSize(2);
//     display.setTextColor(WHITE);
//     display.setCursor(0, 0);
//     display.println("TN(0)");
//     display.setTextSize(1);
//     display.setCursor(10,20);
//     display.print("1: ");
//     display.print(rssi1);
//     display.setCursor(10,30);
//     display.print("2: ");
//     display.print(rssi2);
//     display.setCursor(10,40);
//     display.print("3: ");
//     display.print(rssi3);
//     display.display();
// }
// void button_press(){
//   broadcast = true;
// }
// float to_distance(int input_rssi, int one_m_power, int path_loss_constant){ //RSSI to convert, 1m Power, Path Loss Constant (Between 2 and 4)
//   float output = pow(10,(input_rssi - one_m_power)/(10*path_loss_constant));
//   return output;
// }
// void setup(){
//   //Serial
//   Serial.begin(9600);
//   int test = -83;
//   test = to_distance(test, -73, 2);
//   //Reset OLED
//   pinMode(OLED_RST, OUTPUT);
//   digitalWrite(OLED_RST, LOW);
//   delay(20);
//   digitalWrite(OLED_RST, HIGH);

//   //Initialise OLED
//   Wire.begin(OLED_SDA, OLED_SCL);
//   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {
//     Serial.println(F("OLED not found"));
//     while(1);
//   }
//   updateGUI();
//   //Initialise IMU
//   Wire2.begin(COMPASS_SDA, COMPASS_SCL);
//   compass.setADDR(0x0D);
//   compass.init();
//   compass.setCalibration(-1158, 737, -1612, 388, -1007, 851);

//   //Button
//   pinMode(BUTTON, INPUT);
//   attachInterrupt(BUTTON,button_press,FALLING);

//   //SPI LoRa pins
//   SPI.begin(SCK, MISO, MOSI, SS);
//   //Set LoRa pins
//   LoRa.setPins(SS, RST, DIO0);

//   if (!LoRa.begin(BAND)) {
//     Serial.println("LoRa failed");
//     while (1);
//   }
//   Serial.println("LoRa Initialised");
//   delay(100);
// }
// void loop(){
// int x, y, z, a, b;
// 	char myArray[3];
	
// 	compass.read();
  
// 	x = compass.getX();
// 	y = compass.getY();
// 	z = compass.getZ();
	
// 	a = compass.getAzimuth();
	
// 	b = compass.getBearing(a);

// 	compass.getDirection(myArray, a);
  
  
// 	Serial.print("X: ");
// 	Serial.print(x);

// 	Serial.print(" Y: ");
// 	Serial.print(y);

// 	Serial.print(" Z: ");
// 	Serial.print(z);

// 	Serial.print(" Azimuth: ");
// 	Serial.print(a);

// 	Serial.print(" Bearing: ");
// 	Serial.print(b);

// 	Serial.print(" Direction: ");
// 	Serial.print(myArray[0]);
// 	Serial.print(myArray[1]);
// 	Serial.print(myArray[2]);

// 	Serial.println();

// 	delay(250);
//   if(broadcast){
//     for(int node = 1; node < 4; node++){
//       Serial.println("");
//       Serial.print(node);
//       Serial.print(",");
//       broadcast=false;
//       for(int i = 0; i < 2; i++){
//         waiting=true;
//         LoRa.beginPacket();
//         LoRa.print(String(node));
//         LoRa.endPacket();
//         while(waiting){
//           int packetSize = LoRa.parsePacket();
//           if (packetSize) {
//             if (LoRa.available()) {
//               data = LoRa.readString();
//               if (data  == String(node)){
//                 // rssi[i] = LoRa.packetRssi();
//                 // rssi[4+i] = (data.substring(1, data.length())).toInt();
//                 // Serial.print(rssi[i]);
//                 // Serial.print(",");
//                 // Serial.print(rssi[4+i]);
//                 // Serial.print(",");
//                 Serial.println(LoRa.packetRssi());
//                 waiting = false;
//               }
//               else{
//                 continue;
//               }
//             } 
//           }
//         }
//       }
//       // if(node==1){tx_1 = tx_time;}
//       // else if(node==2){tx_2 = tx_time;}
//       // else if(node==3){tx_3 = tx_time;}
//     }
//     // tx_1 = ((tx_1-calib_1)/(240E6))*299702547*0.5;
//     // tx_2 = ((tx_2-calib_2)/(240E6))*299702547*0.5;
//     // tx_3 = ((tx_3-calib_3)/(240E6))*299702547*0.5;
//     updateGUI();
//   }
// }

