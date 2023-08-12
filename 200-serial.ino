#include <SoftwareSerial.h>

// i make these global because c++ is annoying with pointers :(
char serial_bookend[8];  // what we should discard first on each read
SoftwareSerial serial(1, 3);  // RX, TX


/////////////////////////////////

// flush the buffer. this removes any pesky leftover bits
void serial_flush(Stream &handle, boolean clear_buffer) {
  while ( handle.available() > 0 ) {
    char t = handle.read();
  }
  if ( clear_buffer == true ) {
    global_buffer[0] = '\0';
  }
}


void serial_start() {
  pinMode(1, INPUT);
  pinMode(3, OUTPUT);

  serial_bookend[0] = '\0';

  serial.begin(9600);
  // delay(200);

  serial.listen();
  serial_flush(serial, true);
  // serial.print((char)0);
}


void serial_end() {
  // close the serial connection so we can hand it back to arduino
  serial.end();
}

//boolean serial_check_for_ok() {
//
//  byte ok = 0;
//
//  // we need to read all the headers
//  while ( serial.available() > 0 ) {
//
//    char in_queue = serial.read();
//
//    serial.print("-");
//    serial.print(in_queue);
//    serial.print("~");
//
//    if ( in_queue == 'O' ) {
//      ok = 1;
//    } else if (
//      ( in_queue == 'K' ) &&
//      ( ok == 1 )
//    ) {
//      return true;
//    } else {
//      ok = 0;
//    }
//
//  }    
//
//  return false;
//  
//}




// this allows us to discard anything in the buffer until we find a match
boolean serial_read(Stream &handle) {

  // clear the buffer
  global_buffer[0] = '\0';
  byte global_buffer_read = 0;

  // if there's data to see (this is the initial kick to initiate the loop)
  if ( handle.available() > 0 ) {

    unsigned long limit = 10000;
    unsigned long i = 0;
    boolean received = false;

    // whether we need to discard the bits prior to the bookend...
    byte bookend_size = strlen(serial_bookend);
    byte bookend_used = 0;
    boolean bookend_done = true;
    if ( bookend_size > 0 ) {
      bookend_done = false;
    }

    while ( i < limit ) {

      // if there's a byte waiting in the buffer for us
      if ( handle.available() > 0 ) {

        // discard everything up until a match
        if ( bookend_done == false ) {

          bookend_used = strlen(global_buffer);

          // this just stacks up the string until we reach the size we're looking for
          if ( bookend_used < bookend_size ) {
            global_buffer[bookend_used] = handle.read();
            global_buffer[bookend_used + 1] = '\0';

          // the string must be at the size, we need to shift it all to the left
          } else {
            for ( int j = 1; j < bookend_size; j++ ) {
              global_buffer[j-1] = global_buffer[j];
            }
            global_buffer[bookend_size-1] = handle.read();
            global_buffer[bookend_size] = '\0';
          }

          // now lets see if the serial buffer is equal to the serial bookend that we're looking for...
          if ( strcmp(global_buffer, serial_bookend) == 0 ) {
            // reset the buffer ready for the data
            global_buffer[0] = '\0';

            // break out of the bookend discardy bit
            bookend_done = true;
          }
          
          // reset the loop
          i = 0;
        
        } else {
        
          // let us know something has been received
          received = true;
  
          // read the data...
          byte global_buffer_byte = handle.read();

          // if it's not a newline
          if ( global_buffer_byte != 10 ) {
    
            // if it's not a carriage return - this makes it cr/lf friendly
            if (
              ( global_buffer_byte != 0 ) &&    // null character
              ( global_buffer_byte != 13 ) &&
              ( global_buffer_byte != 2 ) &&
              ( global_buffer_byte != 3 ) &&
              ( global_buffer_read < 100 )
            ) {
  
              // add it to the buffer
              global_buffer[global_buffer_read] = global_buffer_byte;
              global_buffer_read++;
  
            }
    
            // reset the loop
            i = 0;
            
          } else {
            global_buffer[global_buffer_read] = '\0';  // this 'terminates' the string
    
            // cancel the loop
            i = limit;
  
          }
  
        }

      }
            
      i++;

    }
    
    // if we received anything...even if it's just a newline/carriage return
    if ( received == true ) {

      return true;

    }

  }

  // empty the buffer as it could be full of the bookend data
  global_buffer[0] = '\0';
  return false;

}
