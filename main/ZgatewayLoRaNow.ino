/*  
  OpenMQTTGateway  - ESP8266 or Arduino program for home automation 

   Act as a wifi or ethernet gateway between your 433mhz/infrared IR/BLE signal  and a MQTT broker 
   Send and receiving command by MQTT
 
  This gateway enables to:
 - receive MQTT data from a topic and send LoRaNow signal corresponding to the received MQTT data
 - publish MQTT data to a different topic related to received LoRaNow signal

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
#include "User_config.h"

#ifdef ZgatewayLoRaNow

#  include <CayenneLPP.h>
#  include <LoRaNow.h>
#  define MAX_NODES_NUM 80

CayenneLPP lpp(160);
char* unitStrings[] = {"lux", "°C", "%", "G", "hPa", "V", "A", "Hz", "%", "m", "ppm", "W", "m", "kWh", "deg", "°/s", ""};
unsigned long lastMessagetime;
unsigned long seenIds[MAX_NODES_NUM];
uint8_t i_nodes = 0;

bool checkSeen(unsigned long nodeid) {
  for (i_nodes = 0; i_nodes < MAX_NODES_NUM; i_nodes++) {
    if (seenIds[i_nodes] == nodeid) {
      return true;
    }
    if (seenIds[i_nodes] == 0) {
      seenIds[i_nodes] = nodeid;
      return false;
    }
  }
}

uint8_t type2unitidx(uint8_t type) {
  switch (type) {
    case 101:
      return 0;
    case 103:
      return 1;
    case 104:
      return 2;
    case 113:
      return 3;
    case 115:
      return 4;
    case 116:
      return 5;
    case 117:
      return 6;
    case 118:
      return 7;
    case 120:
      return 8;
    case 121:
      return 9;
    case 125:
      return 10;
    case 128:
      return 11;
    case 130:
      return 12;
    case 131:
      return 13;
    case 132:
      return 14;
    case 134:
      return 15;
    default:
      return 16;
  }
}

void setupLoRaNow() {
  //LoRa.setPins(LORANOW_SS, LoRaNow_RST, LoRaNow_DI0);
  LoRaNow.setPins(LORANOW_SS, LORANOW_DI0);
  LoRaNow.setFrequency(LORANOW_BAND);
  LoRa.enableCrc();

  if (!LoRaNow.begin()) {
    Log.error(F("ZgatewayLoRaNow setup failed!" CR));
    while (1)
      ;
  }

  LoRaNow.onMessage(onMessage);
  LoRaNow.gateway();

  Log.notice(F("LORANOW_SCK: %d" CR), LORANOW_SCK);
  Log.notice(F("LORANOW_MISO: %d" CR), LORANOW_MISO);
  Log.notice(F("LORANOW_MOSI: %d" CR), LORANOW_MOSI);
  Log.notice(F("LORANOW_SS: %d" CR), LORANOW_SS);
  Log.notice(F("LORANOW_RST: %d" CR), LORANOW_RST);
  Log.notice(F("LORANOW_DI0: %d" CR), LORANOW_DI0);
  Log.trace(F("ZgatewayLoRaNow setup done" CR));
  lastMessagetime = millis();
  for (uint8_t i = 0; i < MAX_NODES_NUM; i++) {
    seenIds[i] = 0;
  }
}

void endLoRaNow() {
  LoRaNow.end();
}

uint8_t singleTransfer(uint8_t address, uint8_t value) {
  uint8_t response;

  digitalWrite(LORANOW_SS, LOW);

  SPI.beginTransaction(SPISettings(10E6, MSBFIRST, SPI_MODE0));
  SPI.transfer(address);
  response = SPI.transfer(value);
  SPI.endTransaction();

  digitalWrite(LORANOW_SS, HIGH);

  return response;
}

uint8_t readLoRaRegister(uint8_t address) {
  return singleTransfer(address & 0x7f, 0x00);
}

bool checkLoRaWorking() {
  uint8_t reg = readLoRaRegister(0x42);
  if (reg != 0x12) {
    return false;
  } else {
    return true;
  }
}

unsigned long getLastMessageTime() {
  return lastMessagetime;
}

void setLastMessageTime(unsigned long time) {
  lastMessagetime = time;
}

void doLoRaNow() {
  LoRaNow.loop();
}

void setLoRaIdle() {
  LoRa.idle();
}

void setLoRaNowRxMode() {
  LoRa.disableInvertIQ();
  LoRa.receive();
}

uint8_t getLoRaNowState() {
  return LoRaNow.state;
}

void dumpLoRaRegisters(Stream& out) {
  LoRa.dumpRegisters(out);
}

void merge(JsonObject dest, JsonObjectConst src) {
  for (auto kvp : src) {
    dest[kvp.key()] = kvp.value();
  }
}

void createLoRaNowDiscovery(char* type, char* topic, char* name,
                            char* unique_id, char* device_class, char* value_template,
                            char* unit_of_meas, char* device_name) {

  Log.trace(F("CreateDiscoverySensor" CR));
  createDiscovery(type, topic, name, unique_id,
                  will_Topic, device_class, value_template, "", "", unit_of_meas,
                  0, "", "", false, "",
                  device_name, "DIY", "LoraNow node", "" // device name, device manufacturer, device model, device mac
  );
}

void onMessage(uint8_t* buffer, size_t size) {
  lastMessagetime = millis();
  DynamicJsonDocument mqttJsonBuffer(JSON_MSG_BUFFER);
  DynamicJsonDocument valuesJsonBuffer(JSON_MSG_BUFFER);
  CayenneLPP lpp(51);
  JsonArray valJarray = valuesJsonBuffer.to<JsonArray>();
  char topic[sizeof(subjectLoRaNowtoMQTT) + 40];

  lpp.reset();
  lpp.decode(buffer, size, valJarray);

  serializeJson(valuesJsonBuffer, Serial);

  JsonObject LoRaNowdata = mqttJsonBuffer.to<JsonObject>();

  unsigned long id = LoRaNow.id();

  byte count = LoRaNow.count();
  int rssi = LoRa.packetRssi();
  float snr = LoRa.packetSnr();
  long freqErr = LoRa.packetFrequencyError();

  bool seen = checkSeen(id);

  Log.trace(F("Rcv. LoRaNow" CR));
  if (!seen) {
    Log.trace(F("Discovered new node." CR));
  }
  LoRaNowdata["rssi"] = rssi;
  LoRaNowdata["snr"] = snr;
  LoRaNowdata["pferror"] = freqErr;
  LoRaNowdata["packetSize"] = size;
  LoRaNowdata["nodeId"] = (int)id;
  LoRaNowdata["source"] = "LoRaNow";

  sprintf(topic, "%s/%i/STATE", subjectLoRaNowtoMQTT, id);

  if (!seen) {
    char uniqueId[30];
    char deviceName[30];
    sprintf(uniqueId, "%012i_%s", id, "rssi");
    sprintf(deviceName, "LoRaNow node %d", id);
    createLoRaNowDiscovery("sensor", topic, "RSSI",
                           uniqueId, "signal_strength", jsonRSSI,
                           "dBm",
                           deviceName);
    sprintf(uniqueId, "%012i_%s", id, "snr");
    sprintf(deviceName, "LoRaNow node %d", id);
    createLoRaNowDiscovery("sensor", topic, "SNR",
                           uniqueId, "signal_strength", jsonSNR,
                           "dB",
                           deviceName);
  }
  pub(topic, LoRaNowdata);

  for (JsonObject value : valJarray) {
    value["source"] =  "LoRaNow";
    sprintf(topic, "%s/%i/%i/%s", subjectLoRaNowtoMQTT, id,
            value["channel"].as<int>(), value["name"].as<char*>());
    if (!seen) {
      char uniqueId[30];
      char deviceName[30];

      sprintf(uniqueId, "%012i_%i_%s", id,
              value["channel"].as<int>(), value["name"].as<char*>());
      sprintf(deviceName, "LoRaNow node %d", id);

      Log.trace(F("LoraNow Discovery" CR));
      if (value["type"].as<int>() == 142) { // LPP_SWITCH
        char* sensor[8] = {"device_automation", "", "", "", jsonVal, "", "",
                           unitStrings[type2unitidx(value["type"].as<int>())]};
        //component type,name,availability topic,device class,value template,payload on, payload off, unit of measurement

        Log.trace(F("CreateDiscoverySensor" CR));
        //trc(sensor[1]);
        createTriggerDiscovery(topic, uniqueId,
                               deviceName, "DIY", "", "" // device name, device manufacturer, device model, device mac
        );
      } else {
        char* sensor[8] = {"sensor", "", "", "", jsonVal, "", "",
                           unitStrings[type2unitidx(value["type"].as<int>())]};
        //component type,name,availability topic,device class,value template,payload on, payload off, unit of measurement

        Log.trace(F("CreateDiscoverySensor" CR));
        //trc(sensor[1]);
        createDiscovery(sensor[0],
                        topic, (char*)value["name"].as<char*>(), uniqueId,
                        will_Topic, sensor[3], sensor[4],
                        sensor[5], sensor[6], sensor[7],
                        0, "", "", false, "",
                        deviceName, "DIY", "", "" // device name, device manufacturer, device model, device mac
        );
      }
    }
    pub(topic, value);
  }

  //pub(topic, mqttJsonBuffer.as<JsonObject>());
}

#  ifdef jsonReceiving
void MQTTtoLoRaNow(char* topicOri, JsonObject LoRaNowdata) { // json object decoding
  if (cmpToMainTopic(topicOri, subjectMQTTtoLoRaNow)) {
    Log.trace(F("MQTTtoLoRaNow json" CR));
    const char* message = LoRaNowdata["message"];
    int txPower = LoRaNowdata["txpower"] | LORANOW_TX_POWER;
    int spreadingFactor = LoRaNowdata["spreadingfactor"] | LORANOW_SPREADING_FACTOR;
    long int frequency = LoRaNowdata["frequency "] | LORANOW_BAND;
    long int signalBandwidth = LoRaNowdata["signalbandwidth"] | LORANOW_SIGNAL_BANDWIDTH;
    int codingRateDenominator = LoRaNowdata["codingrate"] | LORANOW_CODING_RATE;
    int preambleLength = LoRaNowdata["preamblelength"] | LORANOW_PREAMBLE_LENGTH;
    byte syncWord = LoRaNowdata["syncword"] | LORANOW_SYNC_WORD;
    bool Crc = LoRaNowdata["enablecrc"] | DEFAULT_CRC;
    if (message) {
      LoRa.setTxPower(txPower);
      LoRa.setFrequency(frequency);
      LoRa.setSpreadingFactor(spreadingFactor);
      LoRa.setSignalBandwidth(signalBandwidth);
      LoRa.setCodingRate4(codingRateDenominator);
      LoRa.setPreambleLength(preambleLength);
      LoRa.setSyncWord(syncWord);
      if (Crc)
        LoRa.enableCrc();
      LoRa.beginPacket();
      LoRa.print(message);
      LoRa.endPacket();
      Log.trace(F("MQTTtoLoRaNow OK" CR));
      pub(subjectGTWLoRaNowtoMQTT, LoRaNowdata); // we acknowledge the sending by publishing the value to an acknowledgement topic, for the moment even if it is a signal repetition we acknowledge also
    } else {
      Log.error(F("MQTTtoLoRaNow Fail json" CR));
    }
  }
}
#  endif
#  ifdef simpleReceiving
void MQTTtoLoRaNow(char* topicOri, char* LoRaNowdata) { // json object decoding
  if (cmpToMainTopic(topicOri, subjectMQTTtoLoRaNow)) {
    LoRa.beginPacket();
    LoRa.print(LoRaNowdata);
    LoRa.endPacket();
    Log.notice(F("MQTTtoLoRaNow OK" CR));
    pub(subjectGTWLoRaNowtoMQTT, LoRaNowdata); // we acknowledge the sending by publishing the value to an acknowledgement topic, for the moment even if it is a signal repetition we acknowledge also
  }
}
#  endif
#endif
