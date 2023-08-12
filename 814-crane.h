// https://medium.com/@cn007b/super-simple-php-websocket-example-ea2cd5893575
// sudo a2enmod proxy
// sudo a2enmod proxy_wstunnel
// https://httpd.apache.org/docs/2.4/mod/mod_proxy_wstunnel.html

// *** this library needs to be installed via the library manager
// *** i think the software servo goes to 255, maybe because the attiny85 is only 8 bit
#include <Adafruit_SoftServo.h>
Adafruit_SoftServo servo;

byte servo_pin = 0;

unsigned long socket_last_heard_from = 0;   // this is to get it to refresh each minute with its own timer rather than the bios

void core_init() {

  shift_init(2);

  pinMode(servo_pin, OUTPUT);
  servo.attach(servo_pin);  // attaches the servo on pin 0 to the servo object

  while (true) {

    // clean the register(s)
    shift_buffer[0] = '`';
    shift_buffer[1] = '`';
    shift_buffer[2] = '\0';
    shift_send(2, true);

    // clean the servo
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

                  // f.9.y.
                  // one motor going forward/reverse (f/r)
                  // one motor off
                  // one servo on (1-9)
                  // one servo off
                  // one led on (y)
                  // another led off

                  byte global_buffer_length = strlen(global_buffer);
                  byte shift_bits = 0b00000000;        // the bits that will make up the two characters to shift out
                  byte shift_bits_count = 0;           // how many (out of 8) bits we have worked out so far
                  
                  // start at the end of global buffer
                  for ( byte k = 1; k <= global_buffer_length; k++ ) {

                    byte global_buffer_byte = global_buffer[global_buffer_length - k];

                    // convert the current instruction in to bits
                    // this will total 8 bits eg. 10011011 stored in shift_bits
                    if ( global_buffer_byte == 102 ) {        // f (forward(s))

                      // motor forwards (10)
                      bitSet(shift_bits, shift_bits_count);
                      // shift_bits |= 1 << shift_bits_count;
                      shift_bits_count++;
                      shift_bits_count++; // this leaves the existing bit as 0

                    } else if (
                      ( global_buffer_byte == 114 ) ||        // r (reverse)
                      ( global_buffer_byte == 98 )            // b (backward(s))
                    ) {
                      // motor reverse (01)
                      shift_bits_count++; // this leaves the existing bit as 0
                      bitSet(shift_bits, shift_bits_count);
                      // shift_bits |= 1 << shift_bits_count;
                      shift_bits_count++;

                    } else if ( global_buffer_byte == 115 ) { // s (stop)
                      // motor stop(00)
                      shift_bits_count += 2;  // leave both bits as 0
                    
                    } else if (
                      ( global_buffer_byte >= 49 ) &&         // 1
                      ( global_buffer_byte <= 57 )            // 9
                    ) {
                      // servo
                      servo.write((int)map(global_buffer_byte - 48, 1, 9, 0, 255));
                      delay(15);
                      servo.refresh();

                      // set the bit to 1
                      bitSet(shift_bits, shift_bits_count);
                      // shift_bits |= 1 << shift_bits_count;
                      shift_bits_count++;

                    } else if (
                      ( global_buffer_byte == 121 ) ||        // y (yes)
                      ( global_buffer_byte == 111 )           // o (on)
                    ) {
                      // regular digital output
                      // set the bit to 1
                      bitSet(shift_bits, shift_bits_count);
                      // shift_bits |= 1 << shift_bits_count;
                      shift_bits_count++;

                    } else {
                      // leave the bit as 0
                      shift_bits_count++;
                    
                    }

                    // a tiny bit of error checking
                    if ( shift_bits_count > 8 ) {
                      // discard this round
                      shift_bits = 0;
                      shift_bits_count = 0;

                    // if we have 8 bits stored
                    } else if ( shift_bits_count == 8 ) {
                      
                      // take the higher 4 bits, and turn them in to a character from `a..o
                      shift_buffer[0] = shift_bits >> 4;   // high bits

                      // take the lower 4 bits, and turn them in to a character from `a..o
                      shift_buffer[1] = shift_bits & 0x0F; // low bits

                      // change them in to characters between `a..o
                      shift_buffer[0] += 96;
                      shift_buffer[1] += 96;

                      // close it off
                      shift_buffer[2] = '\0';

                      // shift them out
                      shift_send(2, true);
                      
                      // reset the counter for the next round
                      shift_bits = 0;
                      shift_bits_count = 0;
                    }

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
