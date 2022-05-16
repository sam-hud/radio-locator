/*                           
=================================================
Radio Locator: Beacon Node
v1.0 - May 16 2022
Github: https://github.com/sam-hud/radio-locator

Released under the GNU General Public License v3
[https://www.gnu.org/licenses/gpl-3.0.en.html]
=================================================
*/

#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>

//Node ID - Set before programming device
int id = 1;

//Set Pins
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
#define BAND 866E6 //UK band

//Device setup
void setup(){
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

//Main Loop
void loop(){
  int packetSize = LoRa.parsePacket(); //Check for request message
  if (packetSize) { //If there is a response, continue
    if (LoRa.available()) {
      if (LoRa.readString() == String(id)){ //Check if the data matches this node ID
        LoRa.beginPacket(); //Start communication
        LoRa.print(String(id)); //Send reply message with this node ID
        LoRa.endPacket(); //Stop communication
      }
    }
  }
}