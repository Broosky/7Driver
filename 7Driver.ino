/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Program Name: 7Driver (C)                                                                                               //
// Author: Jeffrey Bednar                                                                                                  //
// Copyright (c) Illusion Interactive, 2011 - 2025.                                                                        //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Date: Saturday, March 30th, 2019 
// Description: 4 x 7 Segment LED Driver With Decimal
//  
// Out 2  Serial Data   74XX595   SER     P14
// Out 4  Serial Clock            SRCLK   P11
// Out 7  Serial Clear            !SRCLR  P10
// Out 8  Demux A       74XX155   A0      P13
// Out 12 Demux B                 A1      P3
// Out 11 Demux EA                !EA     P2
//
// Display segments shifted into registers:
//
//       QA
//   QF      QB
//       QG
//   QE      QC
//       QD      QH (DP)
//
// Display bit alignment:
//
// 0bABCDEFGH
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BUZZ_ENABLE 1
#define BUZZ_DURATION_DEFAULT 35
#define BUZZ_FREQUENCY_DEFAULT 1000
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const uint8_t PIN_SER = 2;
const uint8_t PIN_SRCLK = 4;
const uint8_t PIN_SRCLR = 7;
const uint8_t PIN_A = 8;
const uint8_t PIN_B = 12;
const uint8_t PIN_EA = 11;
const uint8_t PIN_BUZZ = 10;
const uint16_t DELAY = 1000;
const uint16_t DIGITS[] = {
  0b11111100, // "0"
  0b01100000, // "1"
  0b11011010, // "2"
  0b11110010, // "3"
  0b01100110, // "4"
  0b10110110, // "5"
  0b10111110, // "6"
  0b11100000, // "7"
  0b11111110, // "8"
  0b11110110, // "9"
  0b11101110, // "A"
  0b00111110, // "B"
  0b10011100, // "C"
  0b01111010, // "D"
  0b10011110, // "E"
  0b10001110, // "F"
};
const uint16_t SEGMENTS[] = {
  0b10000000, // A
  0b01000000, // B
  0b00100000, // C
  0b00010000, // D
  0b00001000, // E
  0b00000100, // F
  0b00000010, // G
  0b00000001, // .
};
uint16_t a0 = 0, a1 = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup(void) {
  pinMode(PIN_SER, OUTPUT);
  pinMode(PIN_SRCLK, OUTPUT);
  pinMode(PIN_SRCLR, OUTPUT);
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_EA, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void _clock(uint8_t pin) {
  digitalWrite(pin, LOW);
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void clearRegisters(void) {
  // Low to high transistion to clear QA -> QH. This will effectively
  // illuminate all segment LED's because the display is common anode.
  digitalWrite(PIN_SRCLR, LOW);
  digitalWrite(PIN_SRCLR, HIGH);
  // Clock the clear above.
  _clock(PIN_SRCLK);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setDigit(uint8_t digit) {
  switch (digit) {
    case 0: {
        digitalWrite(PIN_A, LOW);
        digitalWrite(PIN_B, LOW);
        break;
      }
    case 1: {
        digitalWrite(PIN_A, HIGH);
        digitalWrite(PIN_B, LOW);
        break;
      }
    case 2: {
        digitalWrite(PIN_A, LOW);
        digitalWrite(PIN_B, HIGH);
        break;
      }
    case 3: {
        digitalWrite(PIN_A, HIGH);
        digitalWrite(PIN_B, HIGH);
        break;
      }
    default: {
        break;
      }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displayEnable(bool enable) {
  if (enable)
    digitalWrite(PIN_EA, LOW); // Inputs accepted.
  else
    digitalWrite(PIN_EA, HIGH); // Inputs ignored.
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void writeOut(uint16_t* data) {
  digitalWrite(PIN_SER, *data & 0x1 ? HIGH : LOW);
  _clock(PIN_SRCLK);
  *data >>= 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t getSegmentBits(char digit, bool commonAnode, bool withDecimal) {
  uint16_t segmentBits = 0;
  if (digit >= 'A')
    segmentBits = DIGITS[digit - 'A' + 10];
  else
    segmentBits = DIGITS[digit - '0'];
  if (commonAnode)
    segmentBits = ~segmentBits;
  if (withDecimal) {
    if (commonAnode)
      segmentBits &= 0b11111110;
    else
      segmentBits |= 0b00000001;
  }
  return segmentBits;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displayWrite(const char* literal, bool commonAnode, uint16_t indexDelay, uint16_t shiftDelay) {
  const char* firstOccurence = NULL;
  char copy[6] = { '\0' };
  short index = -1;
  firstOccurence = strchr(literal, '.');
  index = (short)(firstOccurence - literal);
  strncpy(copy, literal, 6);
  if (index >= 0)
    memmove(&copy[index], &copy[index + 1], strlen(copy) - index);
  for (uint8_t i = 4, j = 0; i > 0; i--, j++) {
    setDigit(i - 1);
    uint16_t data = getSegmentBits(copy[j], commonAnode, j == index - 1 ? true : false);
    for (uint8_t k = 0; k <= 8; k++) {
      delay(shiftDelay);
      writeOut(&data);
    }
    delay(indexDelay);
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void playTone(uint16_t frequency = BUZZ_FREQUENCY_DEFAULT, uint16_t duration = BUZZ_DURATION_DEFAULT) {
  if (BUZZ_ENABLE) {
    tone(PIN_BUZZ, frequency, duration);
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readAnalogPins() {
  a0 = analogRead(A0);
  a1 = analogRead(A1);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop(void) {
  playTone();

  readAnalogPins();

  // Perform a basic POST to verify segment illumination.
  clearRegisters();
  displayEnable(true);
  setDigit(0);
  delay(250);
  setDigit(1);
  delay(250);
  setDigit(2);
  delay(250);
  setDigit(3);
  delay(250);

  playTone();
  char number[6] = { '\0' };
  
  ///*
  // Read seconds since program has started, base 10.
  for (uint16_t i = 0; i < 1000; i++) {
    sprintf(number, "%04d", millis() / 1000);
    displayWrite(number, true, 5, 0);
  }
  //*/

  playTone();

  ///*
  // Read seconds since program has started, base 16.
  for (uint16_t i = 0; i < 1000; i++) {
    sprintf(number, "%04X", millis() / 1000);
    displayWrite(number, true, 5, 0);
  }
  //*/

  playTone();
  
  ///*
  // Read analog input, base 10.
  for (uint16_t i = 0; i < 1000; i++) {
    readAnalogPins();
    uint16_t a0_remapped = map(a0, 0, 1023, 0, 9999);
    uint16_t a1_remapped = map(a1, 0, 1023, 0, 500);
    sprintf(number, "%04d", a0_remapped);
    displayWrite(number, true, 5, 0);
    //displayWrite(number, true, 5, a1_remapped);
  }
  //*/

  playTone();

  ///*
  // Read analog input, base 16.
  for (uint16_t i = 0; i < 1000; i++) {
    readAnalogPins();
    uint16_t a0_remapped = map(a0, 0, 1023, 0, 0xFFFF); // base 16
    uint16_t a1_remapped = map(a1, 0, 1023, 0, 500);
    sprintf(number, "%04X", a0_remapped);
    displayWrite(number, true, 5, 0);
    //displayWrite(number, true, 5, a1_remapped);
  }
  //*/

  playTone();

  ///*
  // Count 0000 -> 9999.
  bool decimalToggle = false;
  for (uint16_t i = 0; i < 9999; i++) {
    readAnalogPins();
    if (i % 25 == 0)
      decimalToggle = !decimalToggle;
    if (decimalToggle)
      sprintf(number, "%04d.", i);
    else
      sprintf(number, "%04d", i);
    displayWrite(number, true, 5, 0);
  }
  //*/

  playTone();

  ///*
  // Count 0000 -> FFFF.
  for (uint16_t i = 0x0000; i < 0xFFFF; i++) {
    readAnalogPins();
    sprintf(number, "%04X", i);
    displayWrite(number, true, 5, 0);
  }
  //*/

  playTone();
  clearRegisters();
  displayEnable(false);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////