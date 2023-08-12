#include <avr/sleep.h>      // Needed for sleep_mode
#include <avr/power.h>
#include <avr/wdt.h>        // Needed to enable/disable watch dog timer
#include <util/atomic.h>
#include <avr/interrupt.h>  // needed to get the ISR function for pin interrupts
#include <TimeLib.h>        // https://playground.arduino.cc/Code/Time/
#include <EEPROM.h>


///////////////////////////////////////////////

// eeprom is divided in to blocks of 100, to match global_buffer.  This allows us to store a few of them!
void at_eeprom_write(byte block) {
  for ( byte i = 0, ilen = strlen(global_buffer); i <= ilen; i++ ) {
    EEPROM.update((block * 100) + i, global_buffer[i]);
  }
}


void at_eeprom_read(byte block) {
  global_buffer[0] = '\0';

  for ( byte i = 0; i < 100; i++ ) {

    // read the byte
    global_buffer[i] = EEPROM.read((block * 100) + i);

    // if we have reached the end...
    if ( global_buffer[i] == '\0' ) {
      // break out of the loop
      break;
    }

  }

}


///////////////////////////////////////////////


// go through each digit, this allows for normal numbers to be stored
unsigned long at_longify_global_buffer() {
  unsigned long output = 0;
  if ( ( strlen(global_buffer) > 0 ) && ( strlen(global_buffer) <= 10 ) ) {   // keeping this below 10 means we won't risk a buffer overflow
    for ( byte i = 0, ilen = strlen(global_buffer); i < ilen; i++ ) {

      // *** MAKE SURE GLOBAL_BUFFER[i] IS A NUMBER
      if ( ( global_buffer[i] >= 48 ) && ( global_buffer[i] <= 57 ) ) {

        // i know this seems crazy, but the pow() function when referencing a variable uses 2k of space, whereas this uses a matter of bytes
        output = (10 * output) + (global_buffer[i] - 48);

      }
    
    }
  }
  return output;
}


void at_shift_global_buffer(byte remove_this_many) {
  // the <= means we'll pick up the \0 at the end also
  for ( byte i = 0, ilen = strlen(global_buffer) - remove_this_many; i <= ilen; i++ ) {
    global_buffer[i] = global_buffer[remove_this_many+i];
  }
}


///////////////////////////////////////////////



// needs the 'Time' library installed
// https://playground.arduino.cc/Code/Time/
// https://github.com/PaulStoffregen/Time
unsigned long at_timestamp = 0;
/*
void at_time(byte what_to_return) {

   // unsigned long tmp;
   if ( what_to_return == 'H' ) { // Returns the hour for the given
     ltoa(hour(at_timestamp), global_buffer, 10); 
   } else if ( what_to_return == 'I' ) {
     ltoa(hourFormat12(at_timestamp), global_buffer, 10);
   } else if ( what_to_return == 'M' ) { // Returns the minute for the given
     ltoa(minute(at_timestamp), global_buffer, 10); 
   } else if ( what_to_return == 'S' ) { // Returns the second for the given
     ltoa(second(at_timestamp), global_buffer, 10); 
   } else if ( what_to_return == 'd' ) { // The day for the given time t
     ltoa(day(at_timestamp), global_buffer, 10); 
   } else if ( what_to_return == 'p' ) {
      if ( isAM(at_timestamp) ) {
        strcpy(global_buffer, "am");
      } else {
        strcpy(global_buffer, "pm");
      }
   } else if ( what_to_return == 'm' ) {
     ltoa(month(at_timestamp), global_buffer, 10); // The month for the given time t
   } else if ( what_to_return == 'G' ) {
     ltoa(year(at_timestamp), global_buffer, 10); // The year for the given time t
   } else if ( what_to_return == 'a' ) {
     strcpy(global_buffer, dayShortStr(at_timestamp));
   } else if ( what_to_return == 'A' ) {
     strcpy(global_buffer, dayStr(at_timestamp));
   } else if ( what_to_return == 'b' ) {
     strcpy(global_buffer, monthShortStr(at_timestamp));
   } else if ( what_to_return == 'B' ) {
     strcpy(global_buffer, monthStr(at_timestamp));
   } else if ( what_to_return == 'u' ) {
     ltoa(weekday(at_timestamp), global_buffer, 10); // Day of the week for the given
     // snprintf(global_buffer, 10, "%dl", weekday(at_timestamp));
   }
   
}
*/

void at_pin_clense() {

  // set all pins as inputs.  this apparently saves power.
  // oh now i have read that they should be outputs with the pullup resistor enabled.  who knows.  https://electronics.stackexchange.com/questions/124723/attiny85-power-consumption-double-expected
  for ( byte i = 0; i <= 4; i++ ) {
    pinMode(i, INPUT);
    // digitalWrite(i, LOW);
  }

}


ISR(WDT_vect) {
  // do nothing
}



void at_watchdog_config(bool deep_sleep, unsigned int how_long) {

      wdt_enable(WDTO_1S);

      if ( deep_sleep ) {
        ADCSRA &= ~(1<<ADEN); //Disable ADC, saves ~230uA
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // turn off everything
        power_all_disable();                   // turn power off to ADC, TIMER 1 and 2, Serial Interface
      } else {
        set_sleep_mode(SLEEP_MODE_IDLE);  // this allows the timer to continue, allowing millis();
      }

      noInterrupts();                        // turn off interrupts as a precaution
    
      //This order of commands is important and cannot be combined
      MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
      WDTCR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
      //WDP3 - WDP2 - WPD1 - WDP0 - time
      // 0      0      0      0      16 ms
      // 0      0      0      1      32 ms
      // 0      0      1      0      64 ms
      // 0      0      1      1      0.125 s
      // 0      1      0      0      0.25 s
      // 0      1      0      1      0.5 s
      // 0      1      1      0      1.0 s
      // 0      1      1      1      2.0 s
      // 1      0      0      0      4.0 s
      // 1      0      0      1      8.0 s
      if ( how_long == 64 ) {
        WDTCR = (0<<WDP3) | (0<<WDP2) | (1<<WDP1) | (0<<WDP0); // set it to 0.064 seconds
      } else if ( how_long == 125 ) {
        WDTCR = (0<<WDP3) | (0<<WDP2) | (1<<WDP1) | (1<<WDP0); // set it to 0.125 seconds
      } else if ( how_long == 250 ) {
        WDTCR = (0<<WDP3) | (1<<WDP2) | (0<<WDP1) | (0<<WDP0); // set it to 0.250 seconds
      } else if ( how_long == 500 ) {
        WDTCR = (0<<WDP3) | (1<<WDP2) | (0<<WDP1) | (1<<WDP0); // 0.5s
      } else {
        WDTCR = (0<<WDP3) | (1<<WDP2) | (1<<WDP1) | (0<<WDP0); // 1 second
      }
      WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
  
      wdt_reset();  // resets the watchdog timer back to 0

      if ( deep_sleep ) {
        // disable the pin change interrupts IMPORTANT!!
        // i have to do it this way because software serial has interrupts that wake it from sleep.
        GIMSK &= ~(1<<PCIE);
      }
      
      // enable interrupts for our sleep THIS IS REQUIRED
      interrupts();
}


void at_watchdog_unconfig(bool deep_sleep) {

      wdt_disable();

      if ( deep_sleep ) {

        // disable interrupts for this bit
        noInterrupts();

        // re-enable the pin interrupts for software serial
        GIMSK |= (1 << PCIE);

        // switch things back on
        power_all_enable();
        ADCSRA |= (1<<ADEN); //Enable ADC

        interrupts();

      }
          
}



// this is just an idle sleep which leaves millis() going, meaning it's not as low power but it does allow
// serial data to keep coming in and continue going back to sleep until the time is up
unsigned long at_when_should_we_stop_idling = 0;
void at_idle(unsigned int how_long) {

      at_when_should_we_stop_idling = millis() + how_long;

      // set the watchdog and sleep mode
      at_watchdog_config(false, how_long);

        while ( at_when_should_we_stop_idling > millis() ) {

          sleep_enable();
        
          // go to sleep
          // sleep_mode();
          sleep_cpu();
        
          //==== here we are asleep ====//
            
          // we have woken back up, disable sleeping
          sleep_disable();

        }
        
      at_watchdog_unconfig(false);

}



// this is a true low power sleep, with no interrupts (including software serial input)
void at_sleep(unsigned int how_long) {

  // set the watchdog and sleep mode
  at_watchdog_config(true, how_long);
  
    sleep_enable();
  
    // go to sleep
    // sleep_mode();
    sleep_cpu();
  
    //==== here we are asleep ====//
      
    // we have woken back up, disable sleeping
    sleep_disable();

  at_watchdog_unconfig(true);

}
