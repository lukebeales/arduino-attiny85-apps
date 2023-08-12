byte ledPin = 0;
byte beepPin = 2;
byte motorPin = 4;

char pager_buffer[9];
char pager_short_uuid[9];   // this is the first 8 characters of the device's uuid


void pager_alert() {
  while ( true ) {
    digitalWrite(motorPin, HIGH);

    for ( byte i = 1; i <= 3; i++ ) {
      digitalWrite(ledPin, HIGH);
      digitalWrite(beepPin, HIGH);
      at_sleep(64);

      digitalWrite(ledPin, LOW);
      digitalWrite(beepPin, LOW);
      at_sleep(64);

      digitalWrite(ledPin, HIGH);
      digitalWrite(beepPin, HIGH);
      at_sleep(64);

      digitalWrite(ledPin, LOW);
      digitalWrite(beepPin, LOW);
      at_sleep(64);

      at_sleep(125);
    }
    
    digitalWrite(motorPin, LOW);
    at_sleep(500);
    at_sleep(250);
  }
}


// checks if there's a matching short uuid in the global_buffer
boolean pager_check() {

  if ( strlen(global_buffer) > 0 ) {

    byte segment = 0;
    byte segment_offset = 0;
    for ( byte i = 0; i <= strlen(global_buffer); i++ ) {
      
      // i think the \0 bit doesn't work yet, maybe serial_read isn't doing the \0 at the end i'm not sure.
      if (
        ( global_buffer[i] == '\0' ) ||   // if we're at the end of the buffer
        ( global_buffer[i] == ',' ) ||    // if we're at the end of an id
        ( segment_offset >= 8 )           // if we're at the max length of an id
      ) {

        // terminate the buffer so it's all kosher
        pager_buffer[segment_offset] = '\0';

        // if there's actually something in the buffer
        if ( strlen(pager_buffer) > 0 ) {

          // see if we match our id
          if ( strcmp(pager_buffer, pager_short_uuid) == 0 ) {
            return true;
          }

        }

        // clear the buffer out
        pager_buffer[0] = '\0';

        // go to the next segment
        segment++;
        segment_offset = 0;

      } else {

        // update the buffer
        pager_buffer[segment_offset] = global_buffer[i];
        segment_offset++;

      }

    }

  }

  return false;

}



void core_init() {

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(beepPin, OUTPUT);
  digitalWrite(beepPin, LOW);
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);

  // allow the esp to wake up first as it throws out a lot of serial data
  at_sleep(1000);
  at_sleep(1000);

  // check if we have an id
  if ( bios_get('*') ) {

    // take just the first 8 characters...
    for ( byte i = 0; i < 8; i++ ) {
      pager_short_uuid[i] = global_buffer[i];
    }
    pager_short_uuid[8] = '\0';


    // load the wifi
    if ( web_open() ) {
  
        // send the headers
        web_page_open(contentLength);
      
          // aren't sending any data, so we don't need to end the data
          // web_data_end_all(false);
    
          web_get_content();
    
        // close the webpage request
        web_page_close();
  
      // close the wifi
      web_close();
  
      // see if we're meant to fire...
      if ( pager_check() ) {

        pager_alert();
  
      }
  
    }

  }
  
  // tell everything to go back to sleep.
  bios_sleep();

}
