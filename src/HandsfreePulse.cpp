#include "HandsfreePulse.h"

HandsfreePulseClass HandsfreePulse;

// Arduino Uno, Duemilanove, Diecimila, LilyPad, Mini, Fio, etc
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega88__) || defined(__AVR_ATmega88__) || defined(__AVR_ATmega48__)

uint8_t HandsfreePulseClass::disconnect(uint8_t pin) {
  uint8_t ErrorCode = 0;
  switch (pin) {
    case 6:
      TCCR0A &= 0b00111111;
      break;
    case 5:
      TCCR0A &= 0b11001111;
      break;
    case 9:
      TCCR1A &= 0b00111111;
      break;
    case 10:
      TCCR1A &= 0b11001111;
      break;
    case 11:
      TCCR2A &= 0b00111111;
      break;
    case 3:
      TCCR2A &= 0b11001111;
      break;
    default:
      ErrorCode = 1;
      break;
  }
  return ErrorCode;
}

uint32_t HandsfreePulseClass::pulse(uint8_t pin, uint32_t duration_in_clock_cycles, uint8_t polarity) {
  uint32_t ActualCycles = 0;
  uint8_t OldTCCRxA;
  uint16_t MaximumDivider = 0;
  uint16_t ValueToWrite;
  if (duration_in_clock_cycles <= 261120UL) { // counter resolution is a minimum of 8 bit - maximum counter value is 255 with a presclaler of 1024
    switch (pin) {
      case 6:
      case 5:
      case 11:
      case 3:
        MaximumDivider = 255;
        break;
      case 9:
      case 10:
        MaximumDivider = 65535UL;
        break;
    }
  }
  else if (duration_in_clock_cycles <= 67107840UL) { // counter resolution is 16 bit - maximum counter value is 65535 with a presclaler of 1024
    switch (pin) {
      case 9:
      case 10:
        MaximumDivider = 65535UL;
        break;
    }
  }
  if (MaximumDivider != 0) {
    bool CanDivideBy_32_128 = false;
    uint8_t prescaler;
    uint16_t factor;
    for (int PrescalerToFit = 1; PrescalerToFit <= 7; PrescalerToFit++) { // 0 is STOP
      switch (pin) {
        case 6:
        case 5:
        case 9:
        case 10:
          switch (PrescalerToFit) {
            case 1:
              factor = 1;
              break;
            case 2:
              factor = 8;
              break;
            case 3:
              factor = 64;
              break;
            case 4:
              factor = 256;
              break;
            case 5:
              factor = 1024;
              break;
          }
          break;
        case 11:
        case 3:
          CanDivideBy_32_128 = true;
          switch (PrescalerToFit) {
            case 1:
              factor = 1;
              break;
            case 2:
              factor = 8;
              break;
            case 3:
              factor = 32;
              break;
            case 4:
              factor = 64;
              break;
            case 5:
              factor = 128;
              break;
            case 6:
              factor = 256;
              break;
            case 7:
              factor = 1024;
              break;
          }
          break;
      }
      ActualCycles = (duration_in_clock_cycles / factor);
      if (CanDivideBy_32_128 == true || (pin != 11 && pin != 3)) {
        if (ActualCycles <= MaximumDivider) {
          ValueToWrite = ActualCycles;
          prescaler = PrescalerToFit;
          break;
        }
      }
    }
    if (ActualCycles < MaximumDivider && factor != 1 && (duration_in_clock_cycles % factor) >= (factor / 2)) { // round it if necessary
      ValueToWrite++;
    }
    switch (pin) { // prescaler reset is common to Timer 0 and Timer 1
      case 6:
        // initialize
        DDRD |= 0b01000000; // set the pin to an output
        TIMSK0 = 0; // disable interrupts
        TCCR0B = 0b00000000; // WGM02 = 01 for CTC on OCR1A and stop the clock
        OldTCCRxA = TCCR0A; // do not disconnect OCxx
        OldTCCRxA &= 0b00111100; // WGM01/WGM00 = 1/0 for CTC on OCR1A
        // set the initial polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b10000000;
        }
        else {
          OldTCCRxA |= 0b11000000;
        }
        TCCR0A = OldTCCRxA; // do not disconnect OCxx
        // reset the prescaler
        GTCCR = 0b10000000;
        GTCCR = 0b10000001;
        GTCCR = 0b00000001;
        // set the desired polarity
        TCCR0B = 0b10000000; // strobe to force a compare match
        OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
        // set the desired polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b11000000;
        }
        else {
          OldTCCRxA |= 0b10000000;
        }
        TCCR0A = OldTCCRxA; // do not disconnect OCxx
        OCR0A = ValueToWrite;
        TCNT0 = 0; // reset for a new pulse
        TCCR0B |= prescaler; // start the pulse
        break;
      case 5:
        // initialize
        DDRD |= 0b00100000; // set the pin to an output
        TIMSK0 = 0; // disable interrupts
        TCCR0B = 0b00000000; // WGM02 = 01 for CTC on OCR1A and stop the clock
        OldTCCRxA = TCCR0A; // do not disconnect OCxx
        OldTCCRxA &= 0b11001100; // WGM01/WGM00 = 1/0 for CTC on OCR1A
        // set the initial polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b00100000;
        }
        else {
          OldTCCRxA |= 0b00110000;
        }
        TCCR0A = OldTCCRxA; // do not disconnect OCxx
        // reset the prescaler
        GTCCR = 0b10000000;
        GTCCR = 0b10000001;
        GTCCR = 0b00000001;
        // set the desired polarity
        TCCR0B = 0b01000000; // strobe to force a compare match
        OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
        // set the desired polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b00110000;
        }
        else {
          OldTCCRxA |= 0b00100000;
        }
        TCCR0A = OldTCCRxA; // do not disconnect OCxx
        OCR0A = ValueToWrite;
        TCNT0 = 0; // reset for a new pulse
        TCCR0B |= prescaler; // start the pulse
        break;
      case 9:
        // initialize
        DDRB |= 0b00000010; // set the pin to an output
        TIMSK1 = 0; // disable interrupts
        TCCR1B = 0b00001000; // WGM13/WGM12 = 0/1 for CTC on OCR1A and stop the clock
        OldTCCRxA = TCCR1A; // do not disconnect OCxx
        OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
        // set the initial polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b10000000;
        }
        else {
          OldTCCRxA |= 0b11000000;
        }
        TCCR1A = OldTCCRxA; // do not disconnect OCxx
        // reset the prescaler
        GTCCR = 0b10000000;
        GTCCR = 0b10000001;
        GTCCR = 0b00000001;
        // set the desired polarity
        TCCR1C = 0b10000000; // strobe to force a compare match
        OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
        // set the desired polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b11000000;
        }
        else {
          OldTCCRxA |= 0b10000000;
        }
        TCCR1A = OldTCCRxA; // do not disconnect OCxx
        OCR1A = ValueToWrite;
        TCNT1 = 0; // reset for a new pulse
        TCCR1B |= prescaler; // start the pulse
        break;
      case 10:
        // initialize
        DDRB |= 0b00000100; // set the pin to an output
        TIMSK1 = 0; // disable interrupts
        TCCR1B = 0b00001000; // WGM13/WGM12 = 0/1 for CTC on OCR1A and stop the clock
        OldTCCRxA = TCCR1A; // do not disconnect OCxx
        OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
        // set the initial polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b00100000;
        }
        else {
          OldTCCRxA |= 0b00110000;
        }
        TCCR1A = OldTCCRxA; // do not disconnect OCxx
        // reset the prescaler
        GTCCR = 0b10000000;
        GTCCR = 0b10000001;
        GTCCR = 0b00000001;
        // set the desired polarity
        TCCR1C = 0b01000000; // strobe to force a compare match
        OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
        // set the desired polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b00110000;
        }
        else {
          OldTCCRxA |= 0b00100000;
        }
        TCCR1A = OldTCCRxA; // do not disconnect OCxx
        OCR1A = ValueToWrite;
        TCNT1 = 0; // reset for a new pulse
        TCCR1B |= prescaler; // start the pulse
        break;
      case 11:
        // initialize
        DDRB |= 0b00001000; // set the pin to an output
        TIMSK2 = 0; // disable interrupts
        TCCR2B = 0b00000000; // WGM22 = 0 for CTC on OCR1A and stop the clock
        OldTCCRxA = TCCR2A; // do not disconnect OCxx
        OldTCCRxA &= 0b00111100; // WGM01/WGM00 = 1/0 for CTC on OCR1A
        // set the initial polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b10000000;
        }
        else {
          OldTCCRxA |= 0b11000000;
        }
        TCCR2A = OldTCCRxA; // do not disconnect OCxx
        // reset the prescaler
        GTCCR = 0b10000000;
        GTCCR = 0b10000010;
        GTCCR = 0b00000010;
        // set the desired polarity
        TCCR2B = 0b10000000; // strobe to force a compare match
        OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
        // set the desired polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b11000000;
        }
        else {
          OldTCCRxA |= 0b10000000;
        }
        TCCR2A = OldTCCRxA; // do not disconnect OCxx
        OCR2A = ValueToWrite;
        TCNT2 = 0; // reset for a new pulse
        TCCR2B |= prescaler; // start the pulse
        break;
      case 3:
        // initialize
        DDRD |= 0b00001000; // set the pin to an output
        TIMSK2 = 0; // disable interrupts
        TCCR2B = 0b00000000; // WGM22 = 0 for CTC on OCR1A and stop the clock
        OldTCCRxA = TCCR2A; // do not disconnect OCxx
        OldTCCRxA &= 0b11001100; // WGM01/WGM00 = 1/0 for CTC on OCR1A
        // set the initial polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b00100000;
        }
        else {
          OldTCCRxA |= 0b00110000;
        }
        TCCR2A = OldTCCRxA; // do not disconnect OCxx
        // reset the prescaler
        GTCCR = 0b10000000;
        GTCCR = 0b10000010;
        GTCCR = 0b00000010;
        // set the desired polarity
        TCCR2B = 0b01000000; // strobe to force a compare match
        OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
        // set the desired polarity
        if (polarity == LOW) {
          OldTCCRxA |= 0b00110000;
        }
        else {
          OldTCCRxA |= 0b00100000;
        }
        TCCR2A = OldTCCRxA; // do not disconnect OCxx
        OCR2A = ValueToWrite;
        TCNT2 = 0; // reset for a new pulse
        TCCR2B |= prescaler; // start the pulse
        break;
    }
    ActualCycles = ValueToWrite;
    ActualCycles *= factor;
  }
  return ActualCycles;
}

void HandsfreePulseClass::pulseFast(uint8_t pin, uint16_t duration_in_cycles, uint8_t prescaler, uint8_t polarity) {
  uint8_t OldTCCRxA;
  switch (pin) { // prescaler reset is common to Timer 0 and Timer 1
    case 6:
      // initialize
      DDRD |= 0b01000000; // set the pin to an output
      if (prescaler == 6 || prescaler == 7) {
        DDRD &= 0b11101111; // set T pin to an input
      }
      TIMSK0 = 0; // disable interrupts
      TCCR0B = 0b00000000; // WGM02 = 01 for CTC on OCR1A and stop the clock
      OldTCCRxA = TCCR0A; // do not disconnect OCxx
      OldTCCRxA &= 0b00111100; // WGM01/WGM00 = 1/0 for CTC on OCR1A
      // set the initial polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b10000000;
      }
      else {
        OldTCCRxA |= 0b11000000;
      }
      TCCR0A = OldTCCRxA; // do not disconnect OCxx
      // reset the prescaler
      GTCCR = 0b10000000;
      GTCCR = 0b10000001;
      GTCCR = 0b00000001;
      // set the desired polarity
      TCCR0B = 0b10000000; // strobe to force a compare match
      OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
      // set the desired polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b11000000;
      }
      else {
        OldTCCRxA |= 0b10000000;
      }
      TCCR0A = OldTCCRxA; // do not disconnect OCxx
      OCR0A = duration_in_cycles;
      TCNT0 = 0; // reset for a new pulse
      TCCR0B |= prescaler; // start the pulse
      break;
    case 5:
      // initialize
      DDRD |= 0b00100000; // set the pin to an output
      if (prescaler == 6 || prescaler == 7) {
        DDRD &= 0b11101111; // set T pin to an input
      }
      TIMSK0 = 0; // disable interrupts
      TCCR0B = 0b00000000; // WGM02 = 01 for CTC on OCR1A and stop the clock
      OldTCCRxA = TCCR0A; // do not disconnect OCxx
      OldTCCRxA &= 0b11001100; // WGM01/WGM00 = 1/0 for CTC on OCR1A
      // set the initial polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b00100000;
      }
      else {
        OldTCCRxA |= 0b00110000;
      }
      TCCR0A = OldTCCRxA; // do not disconnect OCxx
      // reset the prescaler
      GTCCR = 0b10000000;
      GTCCR = 0b10000001;
      GTCCR = 0b00000001;
      // set the desired polarity
      TCCR0B = 0b01000000; // strobe to force a compare match
      OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
      // set the desired polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b00110000;
      }
      else {
        OldTCCRxA |= 0b00100000;
      }
      TCCR0A = OldTCCRxA; // do not disconnect OCxx
      OCR0A = duration_in_cycles;
      TCNT0 = 0; // reset for a new pulse
      TCCR0B |= prescaler; // start the pulse
      break;
    case 9:
      // initialize
      DDRB |= 0b00000010; // set the pin to an output
      if (prescaler == 6 || prescaler == 7) {
        DDRD &= 0b11011111; // set T pin to an input
      }
      TIMSK1 = 0; // disable interrupts
      TCCR1B = 0b00001000; // WGM13/WGM12 = 0/1 for CTC on OCR1A and stop the clock
      OldTCCRxA = TCCR1A; // do not disconnect OCxx
      OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
      // set the initial polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b10000000;
      }
      else {
        OldTCCRxA |= 0b11000000;
      }
      TCCR1A = OldTCCRxA; // do not disconnect OCxx
      // reset the prescaler
      GTCCR = 0b10000000;
      GTCCR = 0b10000001;
      GTCCR = 0b00000001;
      // set the desired polarity
      TCCR1C = 0b10000000; // strobe to force a compare match
      OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
      // set the desired polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b11000000;
      }
      else {
        OldTCCRxA |= 0b10000000;
      }
      TCCR1A = OldTCCRxA; // do not disconnect OCxx
      OCR1A = duration_in_cycles;
      TCNT1 = 0; // reset for a new pulse
      TCCR1B |= prescaler; // start the pulse
      break;
    case 10:
      // initialize
      DDRB |= 0b00000100; // set the pin to an output
      if (prescaler == 6 || prescaler == 7) {
        DDRD &= 0b11011111; // set T pin to an input
      }
      TIMSK1 = 0; // disable interrupts
      TCCR1B = 0b00001000; // WGM13/WGM12 = 0/1 for CTC on OCR1A and stop the clock
      OldTCCRxA = TCCR1A; // do not disconnect OCxx
      OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
      // set the initial polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b00100000;
      }
      else {
        OldTCCRxA |= 0b00110000;
      }
      TCCR1A = OldTCCRxA; // do not disconnect OCxx
      // reset the prescaler
      GTCCR = 0b10000000;
      GTCCR = 0b10000001;
      GTCCR = 0b00000001;
      // set the desired polarity
      TCCR1C = 0b01000000; // strobe to force a compare match
      OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
      // set the desired polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b00110000;
      }
      else {
        OldTCCRxA |= 0b00100000;
      }
      TCCR1A = OldTCCRxA; // do not disconnect OCxx
      OCR1A = duration_in_cycles;
      TCNT1 = 0; // reset for a new pulse
      TCCR1B |= prescaler; // start the pulse
      break;
    case 11:
      // initialize
      DDRB |= 0b00001000; // set the pin to an output
      TIMSK2 = 0; // disable interrupts
      TCCR2B = 0b00000000; // WGM22 = 0 for CTC on OCR1A and stop the clock
      OldTCCRxA = TCCR2A; // do not disconnect OCxx
      OldTCCRxA &= 0b00111100; // WGM01/WGM00 = 1/0 for CTC on OCR1A
      // set the initial polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b10000000;
      }
      else {
        OldTCCRxA |= 0b11000000;
      }
      TCCR2A = OldTCCRxA; // do not disconnect OCxx
      // reset the prescaler
      GTCCR = 0b10000000;
      GTCCR = 0b10000010;
      GTCCR = 0b00000010;
      // set the desired polarity
      TCCR2B = 0b10000000; // strobe to force a compare match
      OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
      // set the desired polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b11000000;
      }
      else {
        OldTCCRxA |= 0b10000000;
      }
      TCCR2A = OldTCCRxA; // do not disconnect OCxx
      OCR2A = duration_in_cycles;
      TCNT2 = 0; // reset for a new pulse
      TCCR2B |= prescaler; // start the pulse
      break;
    case 3:
      // initialize
      DDRD |= 0b00001000; // set the pin to an output
      TIMSK2 = 0; // disable interrupts
      TCCR2B = 0b00000000; // WGM22 = 0 for CTC on OCR1A and stop the clock
      OldTCCRxA = TCCR2A; // do not disconnect OCxx
      OldTCCRxA &= 0b11001100; // WGM01/WGM00 = 1/0 for CTC on OCR1A
      // set the initial polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b00100000;
      }
      else {
        OldTCCRxA |= 0b00110000;
      }
      TCCR2A = OldTCCRxA; // do not disconnect OCxx
      // reset the prescaler
      GTCCR = 0b10000000;
      GTCCR = 0b10000010;
      GTCCR = 0b00000010;
      // set the desired polarity
      TCCR2B = 0b01000000; // strobe to force a compare match
      OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
      // set the desired polarity
      if (polarity == LOW) {
        OldTCCRxA |= 0b00110000;
      }
      else {
        OldTCCRxA |= 0b00100000;
      }
      TCCR2A = OldTCCRxA; // do not disconnect OCxx
      OCR2A = duration_in_cycles;
      TCNT2 = 0; // reset for a new pulse
      TCCR2B |= prescaler; // start the pulse
      break;
  }
}

uint8_t HandsfreePulseClass::pulseClockedWithTpin(uint8_t pin, uint16_t duration_in_T_cycles, uint8_t polarity, uint8_t edge_type) {
  uint8_t ErrorCode = 0;
  if (edge_type == FALLING || edge_type == RISING) {
    if (edge_type == FALLING) {
      edge_type = 6;
    }
    else {
      edge_type = 7;
    }
    switch (pin) {
      case 6:
      case 5:
        if (duration_in_T_cycles > 255) {
          ErrorCode = 1;
        }
        break;
      case 9:
      case 10:
        if (duration_in_T_cycles > 65535UL) {
          ErrorCode = 1;
        }
        break;
      default:
        ErrorCode = 1;
        break;
    }
    if (ErrorCode == 0) {
      uint8_t OldTCCRxA;
      switch (pin) { // prescaler reset is common to Timer 0 and Timer 1
        case 6:
          // initialize
          DDRD &= 0b11101111; // set T pin to an input
          DDRD |= 0b01000000; // set the pin to an output
          TIMSK0 = 0; // disable interrupts
          TCCR0B = 0b00000000; // WGM02 = 0 for CTC on OCR1A and stop the clock
          OldTCCRxA = TCCR0A; // do not disconnect OCxx
          OldTCCRxA &= 0b00111100; // WGM01/WGM00 = 1/0 for CTC on OCR1A
          // set the initial polarity
          if (polarity == LOW) {
            OldTCCRxA |= 0b10000000;
          }
          else {
            OldTCCRxA |= 0b11000000;
          }
          TCCR0A = OldTCCRxA; // do not disconnect OCxx
          // set the desired polarity
          TCCR0B = 0b10000000; // strobe to force a compare match
          OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
          // set the desired polarity
          if (polarity == LOW) {
            OldTCCRxA |= 0b11000000;
          }
          else {
            OldTCCRxA |= 0b10000000;
          }
          TCCR0A = OldTCCRxA; // do not disconnect OCxx
          OCR0A = duration_in_T_cycles;
          TCNT0 = 0; // reset for a new pulse
          TCCR0B |= edge_type; // start the pulse
          break;
        case 5:
          // initialize
          DDRD |= 0b00100000; // set the pin to an output
          DDRD &= 0b11101111; // set T pin to an input
          TIMSK0 = 0; // disable interrupts
          TCCR0B = 0b00000000; // WGM02 = 01 for CTC on OCR1A and stop the clock
          OldTCCRxA = TCCR0A; // do not disconnect OCxx
          OldTCCRxA &= 0b11001100; // WGM01/WGM00 = 1/0 for CTC on OCR1A
          // set the initial polarity
          if (polarity == LOW) {
            OldTCCRxA |= 0b00100000;
          }
          else {
            OldTCCRxA |= 0b00110000;
          }
          TCCR0A = OldTCCRxA; // do not disconnect OCxx
          // set the desired polarity
          TCCR0B = 0b01000000; // strobe to force a compare match
          OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
          // set the desired polarity
          if (polarity == LOW) {
            OldTCCRxA |= 0b00110000;
          }
          else {
            OldTCCRxA |= 0b00100000;
          }
          TCCR0A = OldTCCRxA; // do not disconnect OCxx
          OCR0A = duration_in_T_cycles;
          TCNT0 = 0; // reset for a new pulse
          TCCR0B |= edge_type; // start the pulse
          break;
        case 9:
          // initialize
          DDRB |= 0b00000010; // set the pin to an output
          DDRD &= 0b11011111; // set T pin to an input
          TIMSK1 = 0; // disable interrupts
          TCCR1B = 0b00001000; // WGM13/WGM12 = 0/1 for CTC on OCR1A and stop the clock
          OldTCCRxA = TCCR1A; // do not disconnect OCxx
          OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
          // set the initial polarity
          if (polarity == LOW) {
            OldTCCRxA |= 0b10000000;
          }
          else {
            OldTCCRxA |= 0b11000000;
          }
          TCCR1A = OldTCCRxA; // do not disconnect OCxx
          // set the desired polarity
          TCCR1C = 0b10000000; // strobe to force a compare match
          OldTCCRxA &= 0b00111100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
          // set the desired polarity
          if (polarity == LOW) {
            OldTCCRxA |= 0b11000000;
          }
          else {
            OldTCCRxA |= 0b10000000;
          }
          TCCR1A = OldTCCRxA; // do not disconnect OCxx
          OCR1A = duration_in_T_cycles;
          TCNT1 = 0; // reset for a new pulse
          TCCR1B |= edge_type; // start the pulse
          break;
        case 10:
          // initialize
          DDRB |= 0b00000100; // set the pin to an output
          DDRD &= 0b11011111; // set T pin to an input
          TIMSK1 = 0; // disable interrupts
          TCCR1B = 0b00001000; // WGM13/WGM12 = 0/1 for CTC on OCR1A and stop the clock
          OldTCCRxA = TCCR1A; // do not disconnect OCxx
          OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
          // set the initial polarity
          if (polarity == LOW) {
            OldTCCRxA |= 0b00100000;
          }
          else {
            OldTCCRxA |= 0b00110000;
          }
          TCCR1A = OldTCCRxA; // do not disconnect OCxx
          // set the desired polarity
          TCCR1C = 0b01000000; // strobe to force a compare match
          OldTCCRxA &= 0b11001100; // WGM13/WGM12 = 1/0 for CTC on OCR1A
          // set the desired polarity
          if (polarity == LOW) {
            OldTCCRxA |= 0b00110000;
          }
          else {
            OldTCCRxA |= 0b00100000;
          }
          TCCR1A = OldTCCRxA; // do not disconnect OCxx
          OCR1A = duration_in_T_cycles;
          TCNT1 = 0; // reset for a new pulse
          TCCR1B |= edge_type; // start the pulse
          break;
      }
    }
  }
  else {
    ErrorCode = 1;
  }
  return ErrorCode;
}

void HandsfreePulseClass::ReturnAvailablePrescalers(uint8_t pin, uint16_t *AvailablePrescalers) {
  for (int i = 0; i < AvailablePrescalersPerTimer; i++) {
    AvailablePrescalers[i] = 0;
  }
  switch (pin) {
    case 6: // OC0A
    case 5: // OC0B
    case 9: // OC1A
    case 10: // OC1B
      AvailablePrescalers[0] = 1;
      AvailablePrescalers[1] = 8;
      AvailablePrescalers[2] = 64;
      AvailablePrescalers[3] = 256;
      AvailablePrescalers[4] = 1024;
      break;
    case 11: // OC2A
    case 3: // OC2B
      AvailablePrescalers[0] = 1;
      AvailablePrescalers[1] = 8;
      AvailablePrescalers[2] = 32;
      AvailablePrescalers[3] = 64;
      AvailablePrescalers[4] = 128;
      AvailablePrescalers[5] = 256;
      AvailablePrescalers[6] = 1024;
      break;
  }
}

void HandsfreePulseClass::RestartMillisMicros() { // will start millis() and micros() and reenable delay() functions from the count at the time of disabling
  // values observed after delay() then millis() or micros()
  TCCR0A = 0x03;
  TCNT0 = 0x00; // always incrementing and overflowing
  OCR0A = 0x00;
  OCR0B = 0x00;
  TIFR0 = 0x06;
  TCCR0B = 0x03;
  TIMSK0 = 0x01;
}

uint32_t HandsfreePulseClass::ReturnMaximumDividerValue(uint8_t pin) {
  uint32_t MaximumValue = 0;
  switch (pin) {
    case 6: // OC0A
    case 5: // OC0B
    case 11: // OC2A
    case 3: // OC2A
      MaximumValue = 255;
      break;
    case 9: // OC1A
    case 10: // OC1B
      MaximumValue = 65535UL;
      break;
  }
  return MaximumValue;
}

#else
#error "Unsupported chip, please edit HandsfreePulse library with timer+counter definitions"
#endif