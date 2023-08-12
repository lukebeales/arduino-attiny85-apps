void core_init() {

  /*
    pinMode(3, INPUT);
    pinMode(4, OUTPUT);
    debug.begin(9600);
    delay(200);
  */

/*
  // this is to try and get the wifi to work first go...
      digitalWrite(AT_PIN_SWITCHED, HIGH);
      delay(1500);  // allow things to start...
      web_open();
      web_close();
      digitalWrite(AT_PIN_SWITCHED, LOW);
      delay(1500);  // allow things to start...
  //
*/  

  while( true ) {

      // reset these after sleeping
      pinMode(0, OUTPUT);
      pinMode(2, OUTPUT);


      // this is so we can track how long the measuring/sending takes, and reduce that from the loop delay
      // unsigned long time_switched = millis();

      // set switched on to turn on the wifi chip
      bios_switch(true);

        

        // load the wifi
        web_open();
    
        // setting the bookend here so we avoid any delays and lost data.
        // also web_open wipes serial_bookend clean
        serial_bookend[0] = '\r';
        serial_bookend[1] = '\n';
        serial_bookend[2] = '\r';
        serial_bookend[3] = '\n';
        serial_bookend[4] = '\0';

        
        // this means we'll stay on wifi for a fair while, just making new web requests
        // then we'll leave the loop to reset the wifi cleanly just incase.
        for ( int p = 0; p < 1000; p++ ) {

          // send the headers
          web_page_open(contentLength);
          
          // aren't sending any data, so we don't need to end the data
          // web_data_end_all(false);
  
          // here is where we should see data in theory...
          // if the line starts with +IPX: then it's returned data...
          serial_read(serial);    // throw away the first line, it will be OK or ERROR most likely
          serial_read(serial);    // this is what we want

          // led 1
          if ( global_buffer[0] == '1' ) {
            digitalWrite(0, HIGH);
          } else if ( global_buffer[0] == '0' ) {
            digitalWrite(0, LOW);
          }

          // led 2
          if ( global_buffer[1] == '1' ) {
            digitalWrite(2, HIGH);
          } else if ( global_buffer[1] == '0' ) {
            digitalWrite(2, LOW);
          }
           
          // close the webpage request
          web_page_close();
    
          // wait 2 seconds to see how this goes...
          delay(1000);

        }

        // close the wifi
        web_close();
    
      delay(1500);  // allow things to finish

      bios_switch(false);

  }

}
