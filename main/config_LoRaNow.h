/*  
  OpenMQTTGateway  - ESP8266 or Arduino program for home automation 

   Act as a wifi or ethernet gateway between your 433mhz/infrared IR signal  and a MQTT broker 
   Send and receiving command by MQTT
 
   This files enables to set your parameter for the LoRaNow gateway
  
    Copyright: (c)Florian ROBERT
  
    This file is part of OpenMQTTGateway.
    
    OpenMQTTGateway is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenMQTTGateway is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef config_LoRaNow_h
#define config_LoRaNow_h

extern void setupLoRaNow();
extern void LoRaNowtoMQTT();
extern void doLoRaNow();
extern void endLoRaNow();
extern void setLoRaIdle();
extern void setLoRaNowRxMode();
extern uint8_t getLoRaNowState();
extern unsigned long getLastMessageTime();
extern void getLastMessageTime(unsigned long);
extern uint8_t readLoRaRegister();
extern bool checkLoRaWorking();
extern void dumpLoRaRegisters(Stream& out);
extern void MQTTtoLoRaNow(char* topicOri, char* datacallback);
extern void MQTTtoLoRaNow(char* topicOri, JsonObject& RFdata);
/*----------------------LoRaNow topics & parameters-------------------------*/
#define subjectLoRaNowtoMQTT    "/LoRaNowtoMQTT"
#define subjectMQTTtoLoRaNow    "/commands/MQTTtoLoRaNow"
#define subjectGTWLoRaNowtoMQTT "/LoRaNowtoMQTT"
#define idleCmd    "idle"
#define receiveCmd "receive"
#define stateCmd "state"
#define resetLoRaCmd "resetlora"
#define endLoRaCmd "endlora"
#define dumpRegistersCmd "dumpregisters"
#define readVersionCmd "readversion"


//Default parameters used when the parameters are not set in the json data
#define LORANOW_BAND             433.375E6
#define LORANOW_SIGNAL_BANDWIDTH 125E3
#define LORANOW_TX_POWER         17
#define LORANOW_SPREADING_FACTOR 7
#define LORANOW_CODING_RATE      5
#define LORANOW_PREAMBLE_LENGTH  8
#define LORANOW_SYNC_WORD        0x12
#define DEFAULT_CRC           true

#define repeatLoRaNowwMQTT false // do we repeat a received signal by using mqtt with LoRaNow gateway

/*-------------------PIN DEFINITIONS----------------------*/

// ESP8266 DIY with RFM9x
#define LORANOW_SCK  14 // GPIO5  -- SX1278's SCK
#define LORANOW_MISO 12 // GPIO19 -- SX1278's MISO
#define LORANOW_MOSI 13 // GPIO27 -- SX1278's MOSI
#define LORANOW_SS   15 // GPIO18 -- SX1278's CS
#define LORANOW_RST  4  // GPIO14 -- SX1278's RESET
#define LORANOW_DI0  5  // GPIO26 -- SX1278's IRQ(Interrupt Request)

#endif
