//#include "600-shift.h"
//#include "801-lights.h"

//#include "802-fetch.h"

//#include "804-rfid.h"

//#include "600-shift.h"
//#include "805-digits.h"

//#include "806-gps.h"

//#include "807-random.h"

//#include "808-oled.h"

//#include "809-io_leds.h"  // THIS IS DESIGNED TO BE PERMANENTLY ON
//#include "809-io_sensors.h"

//#include "811-pager.h"

//#include "812-socket.h"

//#include "813-car.h"

#include "600-shift.h"
#include "814-crane.h"


//////////////////////////////////////////////////////////////


void loop() {

  // every app starts with core_init
  core_init();

  // incase the core ends, just sleep forever
  while(true) {
    at_sleep(1000);
  }

}
