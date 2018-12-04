/*******************************************************************************
  Created by Eduardo Contreras @ Electronic Cats 2018
  Based on Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
  PLEASE REFER TO THIS LMIC LIBRARY https://github.com/things-nyc/arduino-lmic
  
  CatWAN USB-Stick
  Example
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

unsigned long previousMillis = 0;


static const PROGMEM u1_t NWKSKEY[16] ={0x98,0xBB,0x8C,0x22,0xFC,0x0D,0x1F,0xB5,0xEF,0x8E,0xA2,0x90,0x76,0xFB,0xCF,0xA7};
static const u1_t PROGMEM APPSKEY[16] ={0xA4,0x98,0xF9,0x32,0xDF,0x71,0xDC,0x0A,0xB2,0xA6,0x15,0x77,0xC4,0xC4,0xCC,0xE7};
static const u4_t DEVADDR =0x077ce77f;


void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

// We will be using Cayenne Payload Format
// For one sensor,
// the general format is channel | type | payload
// payload size depends on type
// here we are using temperature

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;

// Pin mapping for RFM9X
const lmic_pinmap lmic_pins = {
    .nss = SS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RFM_RST,
    .dio = {RFM_DIO0, RFM_DIO1, RFM_DIO2},
};


void onEvent (ev_t ev) {
  switch (ev) {
    case EV_TXCOMPLETE:

      // indicating radio TX complete
      digitalWrite(LED_BUILTIN, LOW);

      SerialUSB.println(F("[LMIC] Radio TX complete (included RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
        SerialUSB.println(F("[LMIC] Received ack"));
      if (LMIC.dataLen) {
        SerialUSB.print(F("[LMIC] Received "));
        SerialUSB.print(LMIC.dataLen);
        SerialUSB.println(F(" bytes of payload"));
        Serial.write(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
      }
      break;

    default:
      SerialUSB.println(F("[LMIC] Unknown event"));
      break;
  }
}

void do_send(osjob_t* j, uint8_t *mydata1, uint16_t len) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    SerialUSB.println(F("[LMIC] OP_TXRXPEND, not sending"));
  } else {
    // Prepare upstream data transmission at the next possible time
    LMIC_setTxData2(1, mydata1, len, 0);
  }
}



void setup() {
  pinMode(LED_BUILTIN,OUTPUT);
  // indicating radio TX complete
  digitalWrite(LED_BUILTIN, LOW);
  SerialUSB.begin(115200);
  SerialUSB.println(F("[INFO] LoRa Demo Node 1 Demonstration"));
  

  os_init();
  LMIC_reset();

  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));

  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);

  for (int channel=0; channel<72; ++channel) {
      LMIC_disableChannel(channel);
    }
  //SF TTN
  /*
      LMIC_enableChannel(8);
      LMIC_enableChannel(9);
      LMIC_enableChannel(10);  //904.3Mhz
      LMIC_enableChannel(11);
      LMIC_enableChannel(12);
      LMIC_enableChannel(13);
      LMIC_enableChannel(14);
      LMIC_enableChannel(15);
      LMIC_enableChannel(65); */
   
  //Home
    
      LMIC_enableChannel(48);
      LMIC_enableChannel(49);
      LMIC_enableChannel(50);
      LMIC_enableChannel(51);
      LMIC_enableChannel(52);
      LMIC_enableChannel(53);
      LMIC_enableChannel(54);
      LMIC_enableChannel(55);
      LMIC_enableChannel(70);
  
  LMIC_setLinkCheckMode(0);
  LMIC_setAdrMode(false);
  LMIC_setDrTxpow(DR_SF7, 14); //SF7

  previousMillis = millis();

}

void loop() {

  if (millis() > previousMillis + TX_INTERVAL * 1000) { //Start Job at every TX_INTERVAL*1000

    getInfoAndSend();
    previousMillis = millis();
  }

  os_runloop_once();
}

void getInfoAndSend() {

  uint8_t len=4;   //Bug of len

  uint8_t mydata[4];
  uint8_t cnt = 0;
  uint8_t ch = 0;
  SerialUSB.println(F("[INFO] Collecting info"));

  float temp = 10.4765;
  SerialUSB.print(F("[INFO] Temperature:")); SerialUSB.println(temp);
  int val = round(temp * 10);
  mydata[cnt++] = ch;
  mydata[cnt++] = 0x67;
  mydata[cnt++] = highByte(val);
  mydata[cnt++] = lowByte(val);
  
    SerialUSB.println(F("[LMIC] Start Radio TX"));
    // indicating start radio TX
    for(int i;i<sizeof(mydata);i++){
      SerialUSB.print(mydata[i]);
    }
    SerialUSB.println();
    digitalWrite(LED_BUILTIN, HIGH);
    do_send(&sendjob, mydata, sizeof(mydata));
  }


float readTemperature() {

  //int a = analogRead(A0);
  int a=1023;
  float R = 1023.0 / ((float)a) - 1.0;
  R = 100000.0 * R;

  float temperature = 1.0 / (log(R / 100000.0) / 4275 + 1 / 298.15) - 273.15; //convert to temperature via datasheet ;

  return temperature;

}
