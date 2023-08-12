void bios_prefix(boolean clear_buffer) {
  serial.flush();
  serial_flush(serial, clear_buffer);
  serial_bookend[0] = '\0';
  serial.print(F("BIOS+"));
}


void bios_switch(boolean should_i) {

//  for ( byte i = 0; i < 2; i++ ) {
    bios_prefix(false);
    if ( should_i == true ) {
      serial.println(F("ON"));
    } else {
      serial.println(F("OFF"));
    }
    serial.flush();
    delay(125);
//  }
  if ( should_i == true ) {
    delay(1000);
    delay(500);
  }
}


boolean bios_ping() {
  bios_prefix(true);
  serial.println(F("PING"));
  serial.flush();
  serial_read(serial);
  if (
    ( global_buffer[0] == 'O' ) &&
    ( global_buffer[1] == 'K' )
  ) {
    return true;
  }
  return false;
}

void bios_random() {
  bios_prefix(true);
  serial.println(F("RND"));
  serial.flush();
  serial_read(serial);
}

void bios_cycle() {
  bios_prefix(true);
  serial.println(F("CYCLE"));
  serial.flush();
  serial_read(serial);
}

void bios_time(bool set = false) {
  if ( set == true ) {
    bios_prefix(false);
    serial.print(F("TIME"));
    serial.print(F("="));
    serial.print(at_timestamp);    
    serial.println("");
  } else {
    bios_prefix(true);
    serial.print(F("TIME"));
    serial.println("");
    serial.flush();
    serial_read(serial);
    // convert it and set it to the at_timestamp value here
    if ( strlen(global_buffer) > 0 ) {
      at_timestamp = at_longify_global_buffer();
      if ( at_timestamp < 1000000000 ) {
        at_timestamp = 0;
      }
    }
  }
  at_sleep(125);
}

void bios_shh() {
  bios_prefix(true);
  serial.println(F("SHH"));
  serial.flush();
  at_sleep(125);
}


void bios_sleep() {
  bios_prefix(true);
  serial.println(F("ZZZ"));
  serial.flush();
}


void bios_get_subroutine(char bios_code, bool fast = false) {
    serial_flush(serial, true);
    bios_prefix(true);
    serial.print(F("GET="));
    serial.println(bios_code);
    serial.flush();
    serial_read(serial);
    if ( ! fast ) {
      at_sleep(125);  // this is to allow for the ESP12F to throw an error after it receives a BIOS+ command
    }
}


bool bios_get(char bios_code, bool fast = false) {
  serial.listen();

/*  

  // don't allow empty values for special bios bits
  if (
    (
      ( bios_code >= 48 ) &&    // 0
      ( bios_code <= 57 )      // 9
    ) ||
    ( bios_code == 42 )    // *
  ) {
*/
    // clear the buffer just incase
    global_buffer[0] = '\0';

    // keep doing it until we have a value
    int i = 0;
    while ( i < 5 ) {
      bios_get_subroutine(bios_code, fast);
      i++;

      // only return a value if it looks ok, particularly with uuids
      if (
        (
          ( bios_code == 42 ) &&
          ( strlen(global_buffer) == 36 ) &&
          ( global_buffer[8] == '-' ) &&
          ( global_buffer[13] == '-' ) &&
          ( global_buffer[18] == '-' ) &&
          ( global_buffer[23] == '-' )
        ) || (
          ( bios_code != 42 ) &&
          ( strlen(global_buffer) > 0 )
        )
      ) {
        return true;
      }
    }
/*
  // otherwise for all other values just run it once and hope for the best.
  } else {

      bios_get_subroutine(bios_code, fast);

  }
*/
  return false;

}
