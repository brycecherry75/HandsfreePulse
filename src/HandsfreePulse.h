#ifndef HandsfreePulse_h
#define HandsfreePulse_h

#include <Arduino.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega88__) || defined(__AVR_ATmega88__) || defined(__AVR_ATmega48__)

#define AvailablePrescalersPerTimer 7
#define FirstRunningPrescaler 1

// Arduino Uno, Duemilanove, Diecimila, LilyPad, Mini, Fio, etc

class HandsfreePulseClass {
  public:
    uint8_t disconnect(uint8_t pin);
    uint32_t pulse(uint8_t pin, uint32_t duration_in_clock_cycles, uint8_t polarity);
    void pulseFast(uint8_t pin, uint16_t duration_in_cycles, uint8_t prescaler, uint8_t polarity); // Timers 0/1 can be externally clocked have prescaler values (0-7) corresponding to STOP 1 8 64 256 1024 T_falling T_rising, otherwise for Timer 2, prescaler values (0-7) correspond to STOP 1 8 32 64 128 256 1024
    uint8_t pulseClockedWithTpin(uint8_t pin, uint16_t duration_in_T_cycles, uint8_t polarity, uint8_t edge_type);
    void ReturnAvailablePrescalers(uint8_t pin, uint16_t *AvailablePrescalers);
    void RestartMillisMicros();
    uint32_t ReturnMaximumDividerValue(uint8_t pin);
};

extern HandsfreePulseClass HandsfreePulse;

    // Arduino Leonardo etc
// #elif defined(__AVR_ATmega32U4__)

    // Arduino Mega etc
// #elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

#else
#error "Unknown chip, please edit HandsfreePulse library with timer+counter definitions"
#endif

#endif