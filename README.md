#HandsfreePulse Library#

HandsfreePulse generates a pulse for a specified duration without tying up the CPU


Revisions:

v1.0.0	First release

FEATURES:

Supported boards (and derivatives): Uno (Mega/Leonardo/ATmega8 support planned)

Ability to stop and resume micros()/millis()/delay() to avoid conflict with these functions and Timer/Counter 0 (delayMicroseconds() is still functional when Timer/Counter 0 is programmed for clock output on OC0x pins)

Pulse functions are self-initializing, but a pulse of the opposite polarity is initally required for setting the idle state of the pin e.g. pin idle state is LOW on reset and to set it to a HIGH idle state, pulse it LOW first

USAGE:

disconnect(pin) - Disconnect a pin from its respective timer - return value is 0 if pin is supported

pulse(pin, duration_in_clock_cycles, polarity) - polarity is HIGH or LOW and the return value will be the clock cycle count rounded down to a compatible prescaler multiple which the maximum is (2 ^ 1024) * ((2 ^ bit_resolution_of_timer_on_pin) - 1); if it is 0, it is not on a valid pin or the clock cycle count is too large

pulseFast(pin, duration_in_clock_cycles, prescaler) - High speed version of pulse - for high speed, NO CHECKS are performed for valid values

pulseClockedWithTpin(pin, duration_in_T_pin_cycles, polarity, edge_type) - polarity is HIGH or LOW and edge_type for T clocking input is RISING or FALLING - return value is 0 if pin can be clocked by an external T input

ReturnAvailablePrescalers(pin, *AvailablePrescalers) - Returns available running prescaler values to uint16_t *AvailablePrescalers array starting from FirstRunningPrescaler constant

RestartMillisMicros(): Disables clock output on pins used by the timer (usually Timer/Counter 0 and its corresponding OCxx pins) and restarts millis()/micros()/delay()

ReturnMaximumDividerValue(pin): Returns a uint32_t maximum divider value on a given pin (result will be 0 if unsupported)

LIMITATIONS:

Due to hardware limitations, pulses of different duration on each pin in a set are not possible and any pin with a common prescaler reset (which does not affect pins clocked with an external T input) will have a longer than expected duration e.g. on an Arduino Uno or derivative, a pulse yet to be completed on Pin 9 will have a longer duration when a pulse is performed on Pin 6.

There will be a number of cycles delay when writing a pulse since hardware does not provide true single pulse generation over an exact number of cycles.

PIN DETAILS - T pin is not used by millis/micros/delay:

Uno (and derivatives): (9/10/(T1: 5) (16 bit), (6/5/(T0: 4)- used by millis/micros/delay))/(11/3) (8 bit) - Pins 6/5/9/10 have a common prescaler reset which is reset using init - for pulseFast, prescaler 0-7 corresponds to stop/1/8/64/256/1024/T_falling/T_rising for Timer 0/1 and for Timer 2, stop/1/8/32/64/128/256/1024

Mega (and derivatives): (5/2/3/T3)/(6/7/8/T4)/(11/12/TOSC1)/(46/45/44/T5: 47) (16 bit), (10/9/T1)/(13/4/T0: 38 - used by millis/micros/delay) (8 bit) - T1/T2/T3/T4/TOSC1 have no traces connected to its headers on the Mega board (official and most of the compatibles)

Leonardo (and derivatives): (11/3/T0: 6 - used by millis/micros/delay) (8 bit), (13/10/T1: 12) (10 bit), 5/(9/10) (16 bit)