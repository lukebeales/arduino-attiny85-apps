// reads serial in and posts it
// theory is to start a second serial port
// read from that
// send the result to the web.

char rfid_tag[20];       // what comes from the serial port

void core_init() {

  SoftwareSerial rfid(0, 2); // RX, TX

  rfid.begin(9600);
  delay(200);

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

  while (true) {

    // reset these after sleeping
    pinMode(0, INPUT);
    pinMode(2, OUTPUT);

    // *** DON'T SET A BOOKEND!  IT'S TO DISCARD DATA UNTIL THE MATCH IS MADE, NOT AS A 'SAVE DATA UP UNTIL A MATCH'!  OMG!
    
    // set this port to be the one to listen on, as software serial can only listen on one at a time.
    // i tried putting this in the 'serial_read' but it didn't like it so this will have to do.
    rfid.listen();
  
    while ( true ) {
      
      // read the second serial thingy here...
      if ( serial_read(rfid) ) {
  
        // if we have some data to send...
        if ( strlen(global_buffer) > 0 ) {
    
          // set switched on to turn on the wifi chip
          bios_switch(true);
        
          // load the wifi
          web_open();

          // the odd characters (ascii 2 & 3) are stripped out at the serial_read bit
          strcpy(rfid_tag, global_buffer);
    
          contentLength += web_data_chunk("rfid", NULL, rfid_tag, NULL, NULL, true);
          contentLength += web_data_end_all(true);

          // send the headers
          web_page_open(contentLength);
        
          // now send the data through
          web_data_chunk("rfid", NULL, rfid_tag, NULL, NULL, false);
          web_data_end_all(false);
                
          // close the webpage request
          web_page_close();

          // empty the buffer out so we don't call it twice.
          serial_flush(rfid);

          // close the wifi
          web_close();
      
          delay(1500);  // allow things to finish
          bios_switch(false);

        }
  
      }
  
    }

  
  }

}
