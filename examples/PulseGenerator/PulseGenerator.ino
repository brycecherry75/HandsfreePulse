/*

   HandsfreePulse demo by Bryce Cherry

   Commands:
   DISCONNECT pin - disconnect a pin from its timer
   PULSE pin duration_in_clock_cycles HIGH/LOW - pulse a pin
   PULSE_FAST pin duration_in_cycles prescaler HIGH/LOW - high speed of the above operation; prescaler is not a division ratio and pin must be previously initialized with INIT
   PULSE_WITH_T pin duration_in_T_cycles HIGH/LOW RISING/FALLING - pulse a pin HIGH/LOW with RISING/FALLING clocking by its T input
   RESTART_MILLIS_MICROS (DELAY_TEST) - disables clock output on pins used by Timer 0 and restores millis()/micros()/delay() - DELAY_TEST tests delay()
   PIN_PARAMETERS pin - displays available prescalers on a given pin

*/

#include <HandsfreePulse.h>

// ensures that the serial port is flushed fully on request
const unsigned long SerialPortRate = 9600;
const byte SerialPortRateTolerance = 5; // percent - increase to 50 for rates above 115200 up to 4000000
const byte SerialPortBits = 10; // start (1), data (8), stop (1)
const unsigned long TimePerByte = ((((1000000ULL * SerialPortBits) / SerialPortRate) * (100 + SerialPortRateTolerance)) / 100); // calculated on serial port rate + tolerance and rounded down to the nearest uS, long caters for even the slowest serial port of 75 bps

const byte commandSize = 50;
char command[commandSize];
const byte FieldSize = 25;

void FlushSerialBuffer() {
  while (true) {
    if (Serial.available() > 0) {
      byte dummy = Serial.read();
      while (Serial.available() > 0) { // flush additional bytes from serial buffer if present
        dummy = Serial.read();
      }
      if (TimePerByte <= 16383) {
        delayMicroseconds(TimePerByte); // delay in case another byte may be received via the serial port
      }
      else { // deal with delayMicroseconds limitation
        unsigned long DelayTime = TimePerByte;
        DelayTime /= 1000;
        if (DelayTime > 0) {
          delay(DelayTime);
        }
        DelayTime = TimePerByte;
        DelayTime %= 1000;
        if (DelayTime > 0) {
          delayMicroseconds(DelayTime);
        }
      }
    }
    else {
      break;
    }
  }
}

void getField (char* buffer, int index) {
  int CommandPos = 0;
  int FieldPos = 0;
  int SpaceCount = 0;
  while (CommandPos < commandSize) {
    if (command[CommandPos] == 0x20) {
      SpaceCount++;
      CommandPos++;
    }
    if (command[CommandPos] == 0x0D || command[CommandPos] == 0x0A) {
      break;
    }
    if (SpaceCount == index) {
      buffer[FieldPos] = command[CommandPos];
      FieldPos++;
    }
    CommandPos++;
  }
  for (int ch = 0; ch < strlen(buffer); ch++) { // correct case of command
    buffer[ch] = toupper(buffer[ch]);
  }
  buffer[FieldPos] = '\0';
}

void setup() {
  Serial.begin(SerialPortRate);
}

void loop() {
  static int ByteCount = 0;
  if (Serial.available() > 0) {
    char inData = Serial.read();
    if (inData != '\n' && ByteCount < commandSize) {
      command[ByteCount] = inData;
      ByteCount++;
    }
    else {
      ByteCount = 0;
      bool ValidField = true;
      char field[FieldSize];
      getField(field, 0);
      if (strcmp(field, "DISCONNECT") == 0) {
        getField(field, 1);
        byte pin = strtoul(field, NULL, 10);
        if (HandsfreePulse.disconnect(pin) != 0) {
          ValidField = false;
        }
      }
      else if (strcmp(field, "PULSE") == 0) {
        getField(field, 1);
        byte pin = strtoul(field, NULL, 10);
        getField(field, 2);
        unsigned long duration_in_clock_cycles = strtoul(field, NULL, 10);
        getField(field, 3);
        if (strcmp(field, "HIGH") == 0 || strcmp(field, "LOW") == 0) {
          byte polarity = LOW;
          if (strcmp(field, "HIGH") == 0) {
            polarity = HIGH;
          }
          duration_in_clock_cycles = HandsfreePulse.pulse(pin, duration_in_clock_cycles, polarity);
          if (duration_in_clock_cycles != 0) {
            Serial.print(F("Clock cycles: "));
            Serial.println(duration_in_clock_cycles);
          }
          else {
            ValidField = false;
          }
        }
        else {
          ValidField = false;
        }
      }
      else if (strcmp(field, "PULSE_FAST") == 0) {
        getField(field, 1);
        byte pin = strtoul(field, NULL, 10);
        getField(field, 2);
        unsigned long duration_in_cycles = strtoul(field, NULL, 10);
        getField(field, 3);
        byte prescaler = strtoul(field, NULL, 10);
        getField(field, 4);
        if (strcmp(field, "HIGH") == 0 || strcmp(field, "LOW") == 0) {
          byte polarity = LOW;
          if (strcmp(field, "HIGH") == 0) {
            polarity = HIGH;
          }
          HandsfreePulse.pulseFast(pin, duration_in_cycles, prescaler, polarity);
        }
        else {
          ValidField = false;
        }
      }
      else if (strcmp(field, "PULSE_WITH_T") == 0) {
        getField(field, 1);
        byte pin = strtoul(field, NULL, 10);
        getField(field, 2);
        unsigned long duration_in_T_cycles = strtoul(field, NULL, 10);
        getField(field, 3);
        if (strcmp(field, "HIGH") == 0 || strcmp(field, "LOW") == 0) {
          byte polarity = LOW;
          if (strcmp(field, "HIGH") == 0) {
            polarity = HIGH;
          }
          getField(field, 4);
          if (strcmp(field, "RISING") == 0 || strcmp(field, "FALLING") == 0) {
            byte edge_type = FALLING;
            if (strcmp(field, "RISING") == 0) {
              edge_type = RISING;
            }
            if (HandsfreePulse.pulseClockedWithTpin(pin, duration_in_T_cycles, polarity, edge_type) != 0) {
              ValidField = false;
            }
          }
          else {
            ValidField = false;
          }
        }
        else {
          ValidField = false;
        }
      }
      else if (strcmp(field, "RESTART_MILLIS_MICROS") == 0) {
        getField(field, 1);
        if (strcmp(field, "DELAY_TEST") == 0) {
          Serial.println(F("Start of delay"));
          delay(5000);
          Serial.println(F("If you see this line immediately after the previous, delay() is inoperative"));
        }
        Serial.print(F("millis() is currently "));
        Serial.println(millis());
        for (int i = 0; i < 500; i++) {
          delayMicroseconds(10000);
        }
        Serial.println(F("If you see this line 5 seconds after the previous, delayMicroseconds() is functional"));
        Serial.print(F("millis() is now "));
        Serial.println(millis());
        HandsfreePulse.RestartMillisMicros();
        delay(5000);
        Serial.print(F("millis() is now "));
        Serial.println(millis());
        Serial.println(F("If you see the above line 5 seconds from the previous line, millis()/micros()/delay() has been successfully restored"));
      }
      else if (strcmp(field, "PIN_PARAMETERS") == 0) {
        getField(field, 1);
        byte pin = strtoul(field, NULL, 10);
        unsigned long MaximumDividerValue = HandsfreePulse.ReturnMaximumDividerValue(pin);
        if (MaximumDividerValue != 0) {
          Serial.print(F("Maximum divider value is "));
          Serial.println(MaximumDividerValue);
          word PrescalerPins[AvailablePrescalersPerTimer];
          HandsfreePulse.ReturnAvailablePrescalers(pin, PrescalerPins);
          Serial.print(F("Prescaler value for pulseFast/available internal prescalers on this pin: "));
          for (int i = 0; i < AvailablePrescalersPerTimer; i++) {
            word temp = PrescalerPins[i];
            if (temp != 0) {
              Serial.print((i + FirstRunningPrescaler));
              Serial.print(F("/"));
              Serial.print(temp);
              Serial.print(F(" "));
            }
            else {
              break;
            }
          }
          Serial.println(F(""));
        }
        else {
          Serial.println(F("Handsfree pulse output is not supported on this pin"));
        }
      }
      else {
        ValidField = false;
      }
      FlushSerialBuffer();
      if (ValidField == true) {
        Serial.println(F("OK"));
      }
      else {
        Serial.println(F("ERROR"));
      }
    }
  }
}