/*
 * 0 = ssid
 * 1 = password
 * 2 = ip
 * 3 = url
 * 4 = host
 */

char secondary_buffer[20];  // i know this takes memory but not much i can do at this point...
uint16_t contentLength = 0;

// this is a desperate attempt to save some bytes
// and much easier than the progmem bits.
bool web_send_atplusc() {
  serial.print(F("AT+C"));
  return true;
}

void web_send_contentdash() {
  serial.print(F("Content-"));
}

void web_send_boundary() {
  serial.print(F("AaB03x"));
}

void web_send_data_prefix(int len) {
  web_send_atplusc();
  serial.print(F("IPSEND="));
  serial.println(len + 2);    // the +2 covers the \n that is sent through also
  // delay(100);
  at_sleep(125);
}


// returns the length of an int in characters (eg. 123 = 3 characters)
// supports up to 1 million.
int length_of_int(int number) {
  byte numberLength = 0;

  int newnum = number;

  // swap the negative to a positive
  if ( number < 0 ) {
    newnum = 0 - number;
    // account for the sign
    numberLength++;
  }

  if ( newnum < 10 ) {
    numberLength += 1;
  } else if ( newnum < 100 ) {
    numberLength += 2;
  } else if ( newnum < 1000 ) {
    numberLength += 3;
  } else if ( newnum < 10000 ) {
    numberLength += 4;
  } else if ( newnum < 100000 ) {
    numberLength += 5;
  } else {
    numberLength += 6;
  }

  return numberLength;
}

void web_disconnect_wifi() {

  // send a few of these just to make sure
  // for ( int i = 0; i < 3; i++ ) {

    // close any lingering connection
    web_send_atplusc();
    serial.println(F("IPCLOSE"));
    // delay(250);
    at_sleep(125);
  
    // disconnect from the wifi
    web_send_atplusc();
    serial.println(F("WQAP"));
    at_sleep(125);
  // }
  
  // delay(250); 
  at_sleep(1000);

}

void web_close() {

  // disconnect from the wifi
  web_disconnect_wifi();
  
}

boolean web_confirm_connection() {
    at_idle(1000);
    serial_read(serial);
    if ( global_buffer[9] == 'I' && global_buffer[10] == 'P' ) {
      return true;
    }
    return false;
}

boolean web_open() {

  // just incase
  contentLength = 0;

  // wait a few seconds...delays everywhere!
  at_sleep(1000);

  // QUERY WHETHER WE ARE ALREADY CONNECTED
/*
  serial.print((char)0);
  serial.flush();
  serial_flush(serial);
  serial_bookend[0] = '\0';
  global_buffer[0] = '\0';
  web_send_atplusc();
  serial.println(F("WJAP?"));
  serial.flush();
  at_sleep(125);
  serial_read(serial);
  // good = +CWJAP:"HRm97WpE","6a:ca:59:25:84:0b",1,-56
  // bad = No AP
  // if we received less than 10 characters then we can assume we're not connected
  // otherwise we can assume we're connected already, no need to do it twice!
  if ( strlen(global_buffer) < 10 ) {
*/
    // this might fix the having to start it twice?
    // bios_count(); // how many times the wifi has switched
    // if it's the first run
    // if ( strcmp(global_buffer, "1") == 0 ) {
      web_disconnect_wifi();
    // }
    
    // https://docs.espressif.com/projects/esp-at/en/latest/Get_Started/Downloading_guide.html
    // firmware tool: https://www.espressif.com/en/support/download/other-tools
    // https://github.com/Edragon/esp_firmware/blob/master/Firmware/AT-espressif/NonOS/V1.7.4-ESP8266_NonOS_AT_Bin_V1.7.4.zip
    // AT+GMR = version info
    // http://room-15.github.io/blog/2015/03/26/esp8266-at-command-reference/
    // https://docs.espressif.com/projects/esp-at/en/latest/AT_Command_Set/Basic_AT_Commands.html#at-test-at-startup
  
    // prevent storing any settings
    // serial.println(F("AT+SYSSTORE=0"));
    // delay(200);
  
    // set the wifi to client mode
    // the 0 tells it not to connect to wifi automatically.  this might save some juice on startup
    // serial.println(F("AT+CWMODE=1,0"));
    web_send_atplusc();
    serial.println(F("WMODE=1"));
    // delay(500);
    at_sleep(250);
    
    // connect to the wifi
    bios_get('0'); // WIFI_SSID
    strcpy(secondary_buffer, global_buffer);
    // delay(200); // delay so things have a chance to clear up.
    at_sleep(250);
    bios_get('1');  // WIFI_PASSWORD
  
    // make sure we're listening to the right serial thingy
    serial.listen();

    web_send_atplusc();
    serial.print(F("WJAP=\""));
    serial.print(secondary_buffer);
    serial.print(F("\",\""));
    serial.print(global_buffer);
    serial.println(F("\""));

/////////////////////////////////////////////////

    boolean connection = false;

    if ( connection == false ) { connection = web_confirm_connection(); }
    if ( connection == false ) { connection = web_confirm_connection(); }
    if ( connection == false ) { connection = web_confirm_connection(); }
    if ( connection == false ) { connection = web_confirm_connection(); }
    if ( connection == false ) { connection = web_confirm_connection(); }

    if ( connection == false ) { connection = web_confirm_connection(); }
    if ( connection == false ) { connection = web_confirm_connection(); }
    if ( connection == false ) { connection = web_confirm_connection(); }
    if ( connection == false ) { connection = web_confirm_connection(); }
    if ( connection == false ) { connection = web_confirm_connection(); }

/*
  // not sure why a simple delay works, but a serial_read doesn't.
  boolean connection = true;
  at_sleep(1000);
  at_sleep(1000);
  at_sleep(1000);
  at_sleep(1000);
  at_sleep(1000);
*/
/////////////////////////////////////////////////

    if ( connection ) {
      at_sleep(1000);
  
      // disable multiple simultaneous connections
      web_send_atplusc();
      serial.println(F("IPMUX=0"));
      // delay(500);
      at_sleep(500);

      return true;

    }
    
    // something failed, close it all
    web_close();
    return false;

}


// all the common bits for opening a page or a socket.  mainly headers
void web_page_socket_open_common() {

  // open a connection to the website
  bios_get('2');  // SERVER_IP
    
  // THIS IS EXTREMELY POWER HUNGRY, IT NEEDS TO DO THIS AS SOON AS POSSIBLE
  web_send_atplusc();
  serial.print(F("IPSTART=\"TCP\",\""));
  serial.print(global_buffer);
  serial.println(F("\",80"));
  serial.flush();

  // delay(1000); // *** allow it ample time to open a connection
  at_sleep(1000);


  bios_get('3');  // SERVER_URL
  if ( strlen(global_buffer) == 0 ) {
    // if no url was specified, just do /
    global_buffer[0] = '\\';   
    global_buffer[1] = '\0';
  }
  
  if ( contentLength > 0 ) {
    web_send_data_prefix(5 + strlen(global_buffer) + 9);
    serial.print(F("POST "));
  } else {
    web_send_data_prefix(4 + strlen(global_buffer) + 9);
    serial.print(F("GET "));
  }
  serial.print(global_buffer);
  serial.println(F(" HTTP/1.1"));
  serial.flush();

  // delay(250);
  at_sleep(250);

  bios_get('4');  // SERVER_HOST

  web_send_data_prefix(6 + strlen(global_buffer));
  serial.print(F("Host: "));
  serial.print(global_buffer);
  serial.println("");
  serial.flush();
  // delay(100);
  at_sleep(125);

  // this is where the server will either accept or reject the request

  bios_get('*');  // DEVICE_UUID

  web_send_data_prefix(8 + strlen(global_buffer));
  serial.print(F("X-UUID: "));
  serial.print(global_buffer);
  serial.println("");
  // delay(100);
  at_sleep(125);

  web_send_data_prefix(25);
  serial.println(F("User-Agent: ResourciBoard"));
  // delay(100);
  at_sleep(125);

}


void web_send_sec_websocket() {
  serial.print(F("Sec-WebSocket-"));
}


// strip the first characters of the socket frame, depending on what serial_read has filtered out already
// this should leave us with squeaky clean data
bool web_socket_filter() {

/*
  // if the esp32 hung up
  if ( strcmp(global_buffer, "CLOSED") == 0 ) {
    return false;
  }
*/

  if (
    ( (byte)global_buffer[0] > 128 ) &&
    ( (byte)global_buffer[0] != 136 )
  ) {
    
    // there should normally be 2 characters at the beginning
    // 0 = frame type
    // 1 = frame size
    byte remove_this_many = 1;
  
    // but the frame size byte might get stripped from serial_read, so we need to see if it matches the length of global_buffer.
    // if it does, we strip it.  if it doesn't, we leave it as it's probably the first byte of data.
    if ( (byte)global_buffer[1] == strlen(global_buffer) - 2 ) {
      remove_this_many = 2;
    }
  
    at_shift_global_buffer(remove_this_many);
  
    return true;

  }
    
  return false;

}


// https://en.wikipedia.org/wiki/WebSocket
// https://medium.com/@cn007b/super-simple-php-websocket-example-ea2cd5893575
void web_socket_open() {
  web_page_socket_open_common();

  web_send_data_prefix(18);
  serial.println(F("Upgrade: websocket"));
  at_sleep(125);

  web_send_data_prefix(19);
  serial.println(F("Connection: Upgrade"));
  at_sleep(125);

  web_send_data_prefix(43);
  web_send_sec_websocket();
  serial.println(F("Key: x3JJHMbDL1EzLkh9GBhXDw=="));
  at_sleep(125);
  
  web_send_data_prefix(39);
  web_send_sec_websocket();
  serial.println(F("Protocol: chat, superchat"));
  at_sleep(125);
  
  web_send_data_prefix(25);
  web_send_sec_websocket();
  serial.println(F("Version: 13"));
  at_sleep(125);
  
  web_send_data_prefix(26);
  serial.println(F("Origin: http://example.com"));
  at_sleep(125);

  web_send_data_prefix(0);
  serial.println("");
}


void web_page_open(uint16_t contentLength) {

  web_page_socket_open_common();

  // tell the server to close this connection after.
  web_send_data_prefix(17);
  serial.println(F("Connection: close"));
  // delay(100);
  at_sleep(125);

  if ( contentLength > 0 ) {
    // let the server know there's data coming and how much
    // this is really bad, but hopefully it will work for now.  i hate c.
    web_send_data_prefix(16 + length_of_int(contentLength));
    web_send_contentdash();
    serial.print(F("Length: "));
    serial.print(contentLength);
    serial.println("");
    // delay(100);
    at_sleep(125);

    web_send_data_prefix(50);
    web_send_contentdash();
    serial.print(F("Type: multipart/form-data; boundary="));
    web_send_boundary();
    serial.println("");
    // delay(100);
    at_sleep(125);
  }

  // tell the bios to ignore everything for a while...
  // bios_shh();

  web_send_data_prefix(0);
  serial.println("");

  
  //at_sleep(125);  // this seems to be required...BUT ONLY FOR OLDER FIRMWARE?

//  serial_flush(serial);

}


// keeps reading serial until we are up to content
bool web_get_content() {

  // disable the really frustrating watchdog reset before doing this
  // otherwise it does really odd behaviour that you'll spend weeks or
  // months thinking it's power issues or odd memory leaks.
  // wdt_disable();

  // flush everything ready to try and find the content...
  serial_flush(serial, true);

  // to know when to stop looking
  unsigned long web_get_content_timeout = millis() + 5000;
  boolean content_is_next = false;
  boolean content_coming = false;
  at_timestamp = 0;

  // this discards everything up to and including these characters, then reads everything following.
  // we need to read it all though
  serial_bookend[0] = '\0';

  // we need to read all the headers
  while ( web_get_content_timeout > millis() ) {

    if ( serial_read(serial) ) {
      if ( strlen(global_buffer) > 0 ) {
  
        // if we have just read the content, let the calling function know.
        // it will be stored in global_buffer
        if ( content_is_next ) {

          // save the time back to the bios so the bios does all the work
          bios_time(true);

          // stick the watchdog back on
          // wdt_enable(WDTO_1S);

          return true;

        }

        // if it says +IPD then we know we're in the goods
        if (
          ( global_buffer[0] == '+' ) &&
          ( global_buffer[1] == 'I' ) &&
          ( global_buffer[2] == 'P' ) &&
          ( global_buffer[3] == 'D' )
        ) {
          content_coming = true;
        }
        
        // if we found the X-Timestamp header, store it for later
        if (
          ( global_buffer[0] == 'X' ) &&
          ( global_buffer[1] == '-' ) &&
          ( global_buffer[2] == 'T' ) &&
          ( global_buffer[3] == 'i' ) &&
          ( global_buffer[4] == 'm' ) &&
          ( global_buffer[5] == 'e' ) &&
          ( global_buffer[11] == ':' )
        ) {

          // store it as a number so we can send it off to the bios after we have the content
          at_shift_global_buffer(13);
          at_timestamp = at_longify_global_buffer();
          
        }

      // if there was nothing in the global buffer, lets assume it's the blank line between the headers and the content
      } else {
        if ( content_coming == true ) {
          content_is_next = true;
        }
      }
    }
  }    

  // stick the watchdog back on
  // wdt_enable(WDTO_1S);

  return false;

}

void web_page_socket_close_common() {
  // give everything a chance to happen first
  // delay(1000);
  at_sleep(1000);

  // close the connection
  web_send_atplusc();
  serial.println(F("IPCLOSE"));
  // delay(250);
  at_sleep(250);
}


void web_socket_close() {
  web_page_socket_close_common();
}

void web_page_close() {
  web_page_socket_close_common();
}



// end all data chunks
int web_data_end_all(bool returnSize) {
  int sizeToReturn = 10 + 2;

  if ( returnSize ) {
    return sizeToReturn;
  } else {
    web_send_data_prefix(10);
    serial.print(F("--"));
    web_send_boundary();
    serial.println(F("--"));
    // delay(100);
    at_sleep(125);
  }
}



int web_data_chunk(char* name, int numbers, char* words, char* filename, char* filedata, bool returnSize) {
        int sizeToReturn = 0;

        // start of data chunk
        sizeToReturn += 8;        // data
        sizeToReturn += 2;        // newline
        if ( ! returnSize ) {
          web_send_data_prefix(8);
          serial.print(F("--"));
          web_send_boundary();
          serial.println("");
          // delay(100);
          at_sleep(125);
        }
          
        // tell the server what the data chunk is (header + name + endquote)
        sizeToReturn += 38;             // header
        sizeToReturn += strlen(name);
        sizeToReturn += 1;              // endquote
        sizeToReturn += 2;              // newline
        if ( ! returnSize ) {
          web_send_data_prefix(38 + strlen(name) + 1);
          web_send_contentdash();
          serial.print(F("Disposition: form-data; name=\""));
          serial.print(name);
          serial.println(F("\""));
          // delay(100);
          at_sleep(125);
        }
        
        sizeToReturn += 2;          // newline
        if ( ! returnSize ) {
          web_send_data_prefix(0);
          serial.println("");
          // delay(100);
          at_sleep(125);
        }
         
        // now pass the value through
        if ( words != NULL ) {
          sizeToReturn += strlen(words);      // data
          sizeToReturn += 2;                  // newline
          if ( ! returnSize ) {
            web_send_data_prefix(strlen(words));
            serial.println(words);
            // delay(100);
            at_sleep(125);
          }
        } else {
          sizeToReturn += length_of_int(numbers);      // data
          sizeToReturn += 2;                           // newline
          if ( ! returnSize ) {
            web_send_data_prefix(length_of_int(numbers));
            serial.println(numbers);
            // delay(100);
            at_sleep(125);
          }
        }

        if ( returnSize ) {
          return sizeToReturn;
        }
      // end of data chunk 

}


/*
  // this is progmem bits, just incase we need to work this out later.

  const PROGMEM char RESOURCIBLE_WIFI_SSID[]      = {"abcdefgh"};
  const PROGMEM char RESOURCIBLE_WIFI_PASSWORD[]  = {"12345678"};
  const PROGMEM char RESOURCIBLE_SERVER_IP[]      = {"12.34.56.78"};
  const PROGMEM char RESOURCIBLE_SERVER_URL[]     = {"/api/analytic"};
  const PROGMEM char RESOURCIBLE_SERVER_HOST[]    = {"dashboard.resourcible.com"};
  

  send_data_prefix(6 + strlen_P(RESOURCIBLE_SERVER_HOST));
  for (byte k = 0; k < strlen(bios_buffer); k++) {
    buffer = pgm_read_byte_near(RESOURCIBLE_SERVER_IP + k);
    serial.print(buffer);
  }
*/
