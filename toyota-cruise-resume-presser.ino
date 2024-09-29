#include <SPI.h>
#include "mcp2515_can.h"
#include "mcp_can.h"
#define CAN_2515

// Define CAN bus pins
const int SPI_CS_PIN = 10;
const int CAN_INTERRUPT_PIN = 2;

bool canMessageAvailable = false;
unsigned long lastRxId;
byte lastRxBuf[8];
byte lastDlc;

mcp2515_can CAN0(SPI_CS_PIN);

#define ACC_CONTROL 0x343
#define PCM_CRUISE 0x1D2

// Define variables to remember the last state of the relevant IDs
bool resumeReady = false;
bool transistorPowered = false;
bool pcmStandstill = false;

unsigned long transistorOnTime = 0;
const unsigned long transistorDuration = 20;

// Define relay control pin
const int BUTTON_TRANSISTOR_PIN = A1;

void setup() {
  pinMode(DIMMER_OUTPUT_RELAY_PIN, OUTPUT);
  digitalWrite(DIMMER_OUTPUT_RELAY_PIN, LOW);
  pinMode(BUTTON_TRANSISTOR_PIN, OUTPUT);
  digitalWrite(BUTTON_TRANSISTOR_PIN, LOW);
  attachInterrupt(digitalPinToInterrupt(CAN_INTERRUPT_PIN), MCP2515_ISR, FALLING);

  bool resumeReady = false;
  bool transistorPowered = false;
  bool pcmStandstill = false;

  if (CAN0.begin(CAN_500KBPS) == CAN_OK) {
    CAN0.init_Mask(0, 0, 0x07FF0000);
    CAN0.init_Mask(1, 0, 0x07FF0000);

    CAN0.init_Filt(0, 0, 0x03430000);
    CAN0.init_Filt(1, 0, 0x01D20000);
    CAN0.init_Filt(2, 0, 0x03430000);
    CAN0.init_Filt(3, 0, 0x01D20000);
    CAN0.init_Filt(4, 0, 0x03430000);
    CAN0.init_Filt(5, 0, 0x01D20000);
  }
}

void MCP2515_ISR() {
  canMessageAvailable = true;
}

void loop() {
  // Check if a CAN message is available
  //  if (CAN0.checkReceive() == CAN_MSGAVAIL) {
  if (canMessageAvailable) {
    canMessageAvailable = false;
    // Now read the CAN message (moved from interrupt to main loop)
    CAN0.readMsgBufID(&lastRxId, &lastDlc, lastRxBuf);
    processCanMessage();          // Process the message
  }

  // Turn off the transistor after transistorDuration
  if (transistorPowered && (millis() - transistorOnTime >= transistorDuration)) {
    PORTC &= ~(1 << PC1);
    transistorPowered = false;  // Reset the flag
  }
}

void processCanMessage() {
  // Process the CAN message from the last CAN read
  if (lastDlc == 8 && lastRxId == ACC_CONTROL) {
    resumeReady = bitRead(lastRxBuf[3], 7);  // Read resume ready state
  }

  if (lastDlc == 8 && lastRxId == PCM_CRUISE) {
    pcmStandstill = ((lastRxBuf[6] & 0xF0) >> 4 == 7 || (lastRxBuf[6] & 0xF0) >> 4 == 11);  // Check CRUISE_STATE
  }

  // Power the transistor briefly if both conditions are true and it's not already powered
  if (resumeReady && pcmStandstill && !transistorPowered) {
    PORTC |= (1 << PC1);          // Power the transistor
    transistorOnTime = millis();  // Record the time when the transistor was turned on
    transistorPowered = true;     // Set the flag indicating the transistor is powered
  }
}
