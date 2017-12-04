/*
  SigFox Simple Weather Station

  This sketch demonstrates the usage of MKRFox1200 as a simple weather station.
  It uses
    the onboard temperature sensor
    HTU21D I2C sensor to get humidity
    Bosch BMP280 to get the barometric pressure
    TSL2561 Light Sensor to get luminosity

  Download the needed libraries from the following links
  http://librarymanager/all#BMP280&Adafruit
  http://librarymanager/all#HTU21D&Adafruit
  http://librarymanager/all#TSL2561&Adafruit
  http://librarymanager/all#adafruit&sensor&abstraction

  Since the Sigfox network can send a maximum of 120 messages per day (depending on your plan)
  we'll optimize the readings and send data in compact binary format

  This example code is in the public domain.
*/

#include <ArduinoLowPower.h>
#include <SigFox.h>
#include "conversions.h"

// Set oneshot to false to trigger continuous mode when you finisched setting up the whole flow
int oneshot = false;

#define STATUS_OK     0
#define STATUS_BMP_KO 1
#define STATUS_HTU_KO 2
#define STATUS_TSL_KO 4

/*
    ATTENTION - the structure we are going to send MUST
    be declared "packed" otherwise we'll get padding mismatch
    on the sent data - see http://www.catb.org/esr/structure-packing/#_structure_alignment_and_padding
    for more details
*/
typedef struct __attribute__ ((packed)) sigfox_message {
  uint8_t status;
  int16_t moduleTemperature;
  uint8_t lastMessageStatus;
} SigfoxMessage;

// stub for message which will be sent
SigfoxMessage msg;

void setup() {

//  if (oneshot == true) {
//    // Wait for the serial
//    Serial.begin(115200);
//    while (!Serial) {}
//  }

  if (!SigFox.begin()) {
    // Something is really wrong, try rebooting
    // Reboot is useful if we are powering the board using an unreliable power source
    // (eg. solar panels or other energy harvesting methods)
    reboot();
  }

  //Send module to standby until we need to send a message
  SigFox.end();

  //if (oneshot == true) {
    // Enable debug prints and LED indication if we are testing
    // SigFox.debug();
  //}

  // SigFox.debug();
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, dummy, CHANGE);
}

void loop() {
  // Every 15 minutes, read all the sensors and send them
  // Let's try to optimize the data format
  // Only use floats as intermediate representaion, don't send them directly
 
  // Start the module
  SigFox.begin();
  // Wait at least 30ms after first configuration (100ms before)
  delay(100);

  // We can only read the module temperature before SigFox.end()
  float temperature = SigFox.internalTemperature();
  // msg.moduleTemperature = convertoFloatToInt16(temperature, 60, -60);
  msg.moduleTemperature = temperature;
  // if (oneshot == true) {
  //  Serial.println("Internal temp: " + String(temperature));
  //}

  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  SigFox.beginPacket();
  SigFox.write((uint8_t*)&msg, 12);

  msg.lastMessageStatus = SigFox.endPacket();

  // if (oneshot == true) {
  //  Serial.println("Status: " + String(msg.lastMessageStatus));
  // }

  SigFox.end();

  //if (oneshot == true) {
    // spin forever, so we can test that the backend is behaving correctly
  //  while (1) {}
  //}

  //Sleep for 15 minutes
  LowPower.sleep(15 * 60 * 1000);
  // LowPower.sleep(2 * 60 * 1000);
}

void dummy() {
  // This function will be called once on device wakeup
  // You can do some little operations here (like changing variables which will be used in the loop)
  // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
}

void reboot() {
  NVIC_SystemReset();
  while (1);
}
