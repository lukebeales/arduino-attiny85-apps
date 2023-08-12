// https://medium.com/@cn007b/super-simple-php-websocket-example-ea2cd5893575
// sudo a2enmod proxy
// sudo a2enmod proxy_wstunnel
// https://httpd.apache.org/docs/2.4/mod/mod_proxy_wstunnel.html

// *** this library needs to be installed via the library manager
// *** i think the software servo goes to 255, maybe because the attiny85 is only 8 bit
#include <Adafruit_SoftServo.h>
Adafruit_SoftServo servo;

byte motor_forward = 2;
byte motor_reverse = 4;
byte servo_pin = 0;

unsigned long socket_last_heard_from = 0;   // this is to get it to refresh each minute with its own timer rather than the bios

void core_init() {

  pinMode(motor_forward, OUTPUT);
  pinMode(motor_reverse, OUTPUT);
  pinMode(servo_pin, OUTPUT);
  servo.attach(servo_pin);  // attaches the servo on pin 0 to the servo object

  while (true) {

    digitalWrite(motor_forward, false);
    digitalWrite(motor_reverse, false);
    
    servo.write(128);
    delay(15);
    servo.refresh();
 
    // let everything cool off, incase we have a camera attached
    at_sleep(1000);
    at_sleep(1000);
    at_sleep(1000);
    at_sleep(1000);
    at_sleep(1000);

    // load the wifi
    if ( web_open() ) {
    
      // send the headers
      web_socket_open();
        
        // aren't sending any data, so we don't need to end the data
        // web_data_end_all(false);
      
          // read the initial headers, set the time, etc.
          web_get_content();

          // filter out the +IPD bit.
          serial_bookend[0] = ':';
          serial_bookend[1] = '\0';

          // keep looping here unless something breaks...
          // then it should loop around the whole thing again like a reset.
          boolean socket_is_connected = true;
          socket_last_heard_from = millis();
          
          while (
            (socket_is_connected) &&
            ( ((1 * 5) - round((millis() - socket_last_heard_from) / 1000)) > 0 )  // this adds about 1k to the code!!!!  wait only 5 seconds before a reset just so the car doesn't drive off in to the sunset
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

                  // steering is between 1 and 9, with 5 being the center.
                  if (
                    ( global_buffer[0] >= 49 ) &&
                    ( global_buffer[0] <= 57 )
                  ) {
                    servo.write((int)map(global_buffer[0] - 48, 1, 9, 0, 255));
                    delay(15);
                  } else {
                    // reset to center
                    servo.write(128);
                    delay(15);
                  }
                  servo.refresh();
                  
                  if ( global_buffer[1] == 'f' ) {
                    digitalWrite(motor_forward, true);
                    digitalWrite(motor_reverse, false);
                  } else if ( global_buffer[1] == 'r' ) {
                    digitalWrite(motor_forward, false);
                    digitalWrite(motor_reverse, true);
                  } else {
                    digitalWrite(motor_forward, false);
                    digitalWrite(motor_reverse, false);
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
