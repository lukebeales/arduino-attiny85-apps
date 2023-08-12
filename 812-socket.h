// https://medium.com/@cn007b/super-simple-php-websocket-example-ea2cd5893575
// sudo a2enmod proxy
// sudo a2enmod proxy_wstunnel
// https://httpd.apache.org/docs/2.4/mod/mod_proxy_wstunnel.html

byte led_red = 2;
byte led_green = 0;
byte led_blue = 4;


unsigned long socket_last_heard_from = 0;   // this is to get it to refresh each minute with its own timer rather than the bios

void core_init() {

  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led_blue, OUTPUT);

  while (true) {

    digitalWrite(led_red, false);
    digitalWrite(led_green, false);
    digitalWrite(led_blue, false);

    // let everything cool off
    at_sleep(1000);
    at_sleep(1000);
    at_sleep(1000);

    // load the wifi
    if ( web_open() ) {
    
digitalWrite(led_red, true);

      // send the headers
      web_socket_open();
        
digitalWrite(led_green, true);

        // aren't sending any data, so we don't need to end the data
        // web_data_end_all(false);
      
          // read the initial headers, set the time, etc.
          web_get_content();

digitalWrite(led_blue, true);

// filter out the +IPD bit.
serial_bookend[0] = ':';
serial_bookend[1] = '\0';


          // keep looping here unless something breaks...
          // then it should loop around the whole thing again like a reset.
          boolean socket_is_connected = true;
          socket_last_heard_from = millis();
          
          while (
            (socket_is_connected) &&
            ( ((1 * 60) - round((millis() - socket_last_heard_from) / 1000)) > 0 )  // this adds about 1k to the code!!!!
          ) {
            if ( serial_read(serial) ) {
              if (
                ( strlen(global_buffer) > 0 ) &&
                ( strlen(global_buffer) < 100 )   // we lose a bit of data because we have to filter out the +IPD stuff for auto reconnect :(
              ) {

                // read the frame...maybe i can make this a common function like web_get_frame or something...

                // *** the issue is serial_read filters out odd characters, so we don't always receive things.
                // first character is the instruction, it will most likely be there as the 64 bit is set
                // second character is the length which we can ignore, it may or may not be there.  if the length is 0, 13, 2 or 3
                // the rest is the data
                
                // *** i think the wisest move for this is if the global_buffer length is <= 5 && != 3 then we need to ignore the second byte, as serial_read will filter it
                // *** we have the length of global_buffer anyway
                if ( web_socket_filter() ) {

                  // so we know the socket is still good
                  socket_last_heard_from = millis();

                  if ( global_buffer[0] == '1' ) {
                    digitalWrite(led_red, true);
                  } else {
                    digitalWrite(led_red, false);
                  }
                  
                  if ( global_buffer[1] == '1' ) {
                    digitalWrite(led_green, true);
                  } else {
                    digitalWrite(led_green, false);
                  }
                  
                  if ( global_buffer[2] == '1' ) {
                    digitalWrite(led_blue, true);
                  } else {
                    digitalWrite(led_blue, false);
                  }

                } else {
                  socket_is_connected = false;
                }
                
              }
            }
          }
      
        // close the webpage request
        web_socket_close();
    
      // close the wifi
      web_close();
    
    }

  }

}
