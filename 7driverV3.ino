/* 4 x 7 Segment LED Driver With Decimal
   Author: Jeff Bednar
   Date: Saturday, March 30th, 2019

   Out 2  Serial Data   74XX595   SER     P14
   Out 4  Serial Clock            SRCLK   P11
   Out 7  Serial Clear            !SRCLR  P10
   Out 8  Demux A       74XX155   A0      P13
   Out 12 Demux B                 A1      P3
   Out 11 Demux EA                !EA     P2

   Display segments shifted into registers:

         QA
     QF      QB
         QG
     QE      QC
         QD      QH (DP)

   Display bit alignment:

   0bABCDEFGH
*/
#define BUZZ_ENABLE 1
#define BUZZ_DURATION_DEFAULT 35
#define BUZZ_FREQUENCY_DEFAULT 1000

const byte PIN_SER = 2;
const byte PIN_SRCLK = 4;
const byte PIN_SRCLR = 7;
const byte PIN_A = 8;
const byte PIN_B = 12;
const byte PIN_EA = 11;
const byte PIN_BUZZ = 10;
const short DELAY = 1000;
const unsigned short DIGITS[] = {
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
const unsigned short SEGMENTS[] = {
  0b10000000, // A
  0b01000000, // B
  0b00100000, // C
  0b00010000, // D
  0b00001000, // E
  0b00000100, // F
  0b00000010, // G
  0b00000001, // .
};
unsigned short a0 = 0, a1 = 0;

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

void _clock(const byte pin) {
  digitalWrite(pin, LOW);
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}

void clearRegisters(void) {
  // Low to high transistion to clear QA -> QH. This will effectively
  // illuminate all segment LED's because the display is common anode.
  digitalWrite(PIN_SRCLR, LOW);
  digitalWrite(PIN_SRCLR, HIGH);
  // Clock the clear above.
  _clock(PIN_SRCLK);
}

void setDigit(const byte digit) {
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

void displayEnable(const bool enable) {
  if (enable)
    digitalWrite(PIN_EA, LOW); // Inputs accepted
  else
    digitalWrite(PIN_EA, HIGH); // Inputs ignored
}

void writeOut(const byte pin, unsigned short* data) {
  digitalWrite(PIN_SER, *data & 0x1 ? HIGH : LOW);
  _clock(PIN_SRCLK);
  *data >>= 1;
}

unsigned short getSegmentBits(char digit, bool commonAnode, bool withDecimal) {
  unsigned short segmentBits = 0;
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

void displayWrite(const char* literal, const bool commonAnode, const unsigned short indexDelay, const unsigned short shiftDelay) {
  const char* firstOccurence = NULL;
  char copy[6] = { '\0' };
  short index = -1;
  firstOccurence = strchr(literal, '.');
  index = (short)(firstOccurence - literal);
  strncpy(copy, literal, 6);
  if (index >= 0)
    memmove(&copy[index], &copy[index + 1], strlen(copy) - index);
  for (byte i = 4, j = 0; i > 0; i--, j++) {
    setDigit(i - 1);
    unsigned short data = getSegmentBits(copy[j], commonAnode, j == index - 1 ? true : false);
    for (byte k = 0; k <= 8; k++) {
      delay(shiftDelay);
      writeOut(PIN_SER, &data);
    }
    delay(indexDelay);
  }
}

void playTone(unsigned short frequency = BUZZ_FREQUENCY_DEFAULT, unsigned short duration = BUZZ_DURATION_DEFAULT) {
  if (BUZZ_ENABLE) {
    tone(PIN_BUZZ, frequency, duration);
  }
}

void readAnalogPins() {
  a0 = analogRead(A0);
  a1 = analogRead(A1);
}

void loop(void) {
  playTone();

  readAnalogPins();

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
  // Read seconds since program has started, base 10
  for (unsigned short i = 0; i < 1000; i++) {
    sprintf(number, "%04d", millis() / 1000);
    displayWrite(number, true, 5, 0);
  }
  //*/

  playTone();

  ///*
  // Read seconds since program has started, base 16
  for (unsigned short i = 0; i < 1000; i++) {
    sprintf(number, "%04X", millis() / 1000);
    displayWrite(number, true, 5, 0);
  }
  //*/

  playTone();
  
  ///*
  // Read analog input, base 10
  for (unsigned short i = 0; i < 1000; i++) {
    readAnalogPins();
    unsigned short a0_remapped = map(a0, 0, 1023, 0, 9999);
    unsigned short a1_remapped = map(a1, 0, 1023, 0, 500);
    sprintf(number, "%04d", a0_remapped);
    displayWrite(number, true, 5, 0);//a1_remapped);
  }
  //*/

  playTone();

  ///*
  // Read analog input, base 16
  for (unsigned short i = 0; i < 1000; i++) {
    readAnalogPins();
    unsigned short a0_remapped = map(a0, 0, 1023, 0, 0xFFFF); // base 16
    unsigned short a1_remapped = map(a1, 0, 1023, 0, 500);
    sprintf(number, "%04X", a0_remapped);
    displayWrite(number, true, 5, 0);//a1_remapped);
  }
  //*/

  playTone();

  ///*
  // Count 0000 -> 9999
  bool decimalToggle = false;
  for (unsigned short i = 0; i < 9999; i++) {
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
  // Count 0000 -> FFFF
  for (unsigned short i = 0x0000; i < 0xFFFF; i++) {
    readAnalogPins();
    sprintf(number, "%04X", i);
    displayWrite(number, true, 5, 0);
  }
  //*/

  playTone();
  clearRegisters();
  displayEnable(false);
}
