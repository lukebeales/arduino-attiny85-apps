// #define MCUCSR MCUSR

void setup() {

  // turn off the watchdog as all it does is cause issues
  wdt_disable();

  at_pin_clense();

  // prepare us for the bios and wifi
  serial_start();

/*
  // find out why a reset happened.  found here:
  // https://stackoverflow.com/a/23377949/3417896
  if(MCUCSR & (1<<PORF )) serial.println(F("Power-on reset."));
  if(MCUCSR & (1<<EXTRF)) serial.println(F("External reset!"));
  if(MCUCSR & (1<<BORF )) serial.println(F("Brownout reset!"));
  if(MCUCSR & (1<<WDRF )) serial.println(F("Watchdog reset!"));
  // if(MCUCSR & (1<<JTRF )) serial.println(F("JTAG reset!"));
  MCUCSR = 0;
*/

  // don't think we need this yet.
//  serial.println(F("-init-"));
//  delay(200);
  
}
