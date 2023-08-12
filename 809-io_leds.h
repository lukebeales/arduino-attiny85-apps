byte led_red = 2;  // red
byte led_yellow = 0;  // yellow
byte led_green = 4;  // green

void core_init() {

  pinMode(led_red, OUTPUT);
  pinMode(led_yellow, OUTPUT);
  pinMode(led_green, OUTPUT);
  digitalWrite(led_red, false);
  digitalWrite(led_yellow, false);
  digitalWrite(led_green, false);

  while(true) {

    // check if the bios is awake so we can do serial commands
    if ( bios_ping() ) {

      // wait for things to start
      at_sleep(1000);
      at_sleep(1000);

      // load the wifi
      if ( web_open() ) {
    
        // send the headers
        web_page_open(contentLength);

          // at_sleep(125);
    
          web_get_content();

          if ( strlen(global_buffer) > 0 ) {
        
            // reset all the leds to off.
            digitalWrite(led_red, false);
            digitalWrite(led_yellow, false);
            digitalWrite(led_green, false);
      
            for ( byte i = 0; i <= strlen(global_buffer); i++ ) {
              
              if ( global_buffer[i] == '2' ) {
                digitalWrite(led_red, true);
              } else if ( global_buffer[i] == '0' ) {
                digitalWrite(led_yellow, true);
              } else if ( global_buffer[i] == '4' ) {
                digitalWrite(led_green, true);
              }
      
            }
      
          }

        // close the webpage request
        web_page_close();
  
        // close the wifi
        web_close();

      }
    
      bios_sleep();

    }

    at_sleep(1000);

  }
  
}
