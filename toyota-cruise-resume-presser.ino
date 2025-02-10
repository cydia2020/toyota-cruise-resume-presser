#include <SPI.h>
#include <mcp_canbus.h>

// Define CAN bus pins
const int SPI_CS_PIN = 9;
const int CAN_INTERRUPT_PIN = 11;

// Define relay control pin
const int RESUME_BUTTON_TRANSISTOR_PIN = 10;

volatile bool canMessageAvailable = false;
unsigned long lastRxId;
byte lastRxBuf[8];

MCP_CAN CAN0(SPI_CS_PIN);

#define PCM_CRUISE_SM 0x399

// Define state machine
enum TransistorState { TRANSISTOR_OFF,
                       TRANSISTOR_ON };
TransistorState transistorState = TRANSISTOR_OFF;

// Variables tracking CAN data
bool pcmResumeReady = false;

void setup() {
  pinMode(RESUME_BUTTON_TRANSISTOR_PIN, OUTPUT);
  digitalWrite(RESUME_BUTTON_TRANSISTOR_PIN, LOW);
  pinMode(CAN_INTERRUPT_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CAN_INTERRUPT_PIN), MCP2515_ISR, FALLING);

  if (CAN0.begin(CAN_500KBPS) == CAN_OK) {
    CAN0.init_Mask(0, 0, 0x7FF);
    CAN0.init_Filt(0, 0, 0x399);
    CAN0.init_Filt(1, 0, 0x399);
    CAN0.init_Mask(1, 0, 0x7FF);
    CAN0.init_Filt(2, 0, 0x399);
    CAN0.init_Filt(3, 0, 0x399);
    CAN0.init_Filt(4, 0, 0x399);
    CAN0.init_Filt(5, 0, 0x399);
  }
}

void MCP2515_ISR() {
  canMessageAvailable = true;
}

void loop() {
  if (canMessageAvailable) {
    canMessageAvailable = false;
    CAN0.readMsgBufID(&lastRxId, nullptr, lastRxBuf);
    processCanMessage();
  }
  updateTransistorState();
}

void processCanMessage() {
  switch (lastRxId) {

    case PCM_CRUISE_SM:
      pcmResumeReady = (lastRxBuf[1] & 0x0F) == 10;
      break;
  }
}

void updateTransistorState() {
  static bool lastTransistorState = LOW;

  transistorState = pcmResumeReady ? TRANSISTOR_ON : TRANSISTOR_OFF;

  if (transistorState != lastTransistorState) {
    if (transistorState == TRANSISTOR_ON) {
      digitalWrite(RESUME_BUTTON_TRANSISTOR_PIN, HIGH);
    } else {
      digitalWrite(RESUME_BUTTON_TRANSISTOR_PIN, LOW);
    }
    lastTransistorState = transistorState;
  }
}
