// drives shift registers depending on what it receives from the web.
// s = segments
// r = registers per segment


byte digits_segments = 1;         // how many segments there are
byte digits_segment_registers = 2;   // how many registers per segment (eg. 16 segment displays have 2)
unsigned long digits_time_started = 0;   // this is to get it to refresh each minute with its own timer rather than the bios
char digits_buffer[100];

void core_init() {


  // 'init' the shift register
  shift_init(3);

  // clear the digits buffer.  unsure if this is needed but what the hey!
  digits_buffer[0] = '\0';

/*
  // show that we are thinking
  shift_buffer[0] = '-';
  shift_buffer[1] = '\0';
  shift_translate(digits_segment_registers);
  for ( byte s = 0; s < digits_segments; s++ ) {
    shift_send(3, true);
  }
*/

  while( true ) {

    // segments
    if ( bios_get('s') ) {
      if ( global_buffer[0] > 48 ) {
        digits_segments = global_buffer[0] - 48;
      }
    }

    // how many registers per segment
    if ( bios_get('r') ) {
      if ( global_buffer[0] > 48 ) {
        digits_segment_registers = global_buffer[0] - 48;
      }
    }

    if (
      ( digits_segments > 0 ) &&
      ( digits_segments <= 10 ) &&
      ( digits_segment_registers > 0 ) &&
      ( digits_segment_registers <= 10 )
    ) {

      digits_time_started = millis();

      // set switched on to turn on the wifi chip
      bios_switch(true);
  
        // load the wifi
        if ( web_open() ) {

            // send the headers
            web_page_open(contentLength);
            
            web_get_content();
  
            // if we have a valid looking length returned...
            if (
              ( strlen(global_buffer) > 0 ) &&
              ( strlen(global_buffer) < 100 )
            ) {
        
              // copy the contents of global_buffer to digits_buffer
              strcpy(digits_buffer, global_buffer);
  
            }    
  
            // close the webpage request
            web_page_close();
      
          // close the wifi
          web_close();
        }
        
        bios_switch(false);
  
        if ( strlen(digits_buffer) > 0 ) {
  
          // some initial settings to play nice with the loop...
          byte buffer_offset = 127;  // how far in to the buffer we are reading
          byte buffer_chunk_size = 0;     // how big the current buffer chunk is
          int buffer_scroll_offset = 127;   // how far in to the current chunk we are
          boolean busy = true;
  
          ///////////////////////////////////////
          // here's where we shift the leds
          ///////////////////////////////////////
  
            // loop for the rest of 60 seconds...
            while (
              ( ((1 * 60) - round((millis() - digits_time_started) / 1000)) > 0 ) ||
              ( busy )
            ){

              // figure out what we're showing...
                if ( buffer_scroll_offset > buffer_chunk_size + digits_segments ) {
  
                  // move to the next 'chunk'
                  buffer_offset = buffer_offset + buffer_chunk_size + 1;  // the 1 is because buffer_chunk_size doesn't start at 0.
                  if ( buffer_offset >= strlen(digits_buffer) ) {
                    buffer_offset = 0;
                  }
  
                  // find out how big the current 'chunk' is
                  buffer_scroll_offset = 0;
                  buffer_chunk_size = 0;
                  for ( byte c = buffer_offset; c < strlen(digits_buffer); c++ ) {
                    if ( (char)digits_buffer[c] != ';' ) {
                      buffer_chunk_size++;
                    } else {
                      break;
                    }
                  }
                  
                  // if we need to scroll
                  if ( buffer_chunk_size > digits_segments ) {
                    buffer_scroll_offset = 0 - digits_segments;
                    busy = true;
                  } else {
                    busy = false;
                  }
                }
              //
  
                
              shift_latch(false);
              
              // this one is backwards
              // we read the right-most segment first, because that gets pushed along the chain as we go
              for ( byte k = 1; k <= digits_segments; k++ ) {
        
                // because we're receiving numbers/letters, we only need to populate the first byte of the shift buffer
                // if we're scrolling outside of any normal value, just set a space...
                int z = buffer_scroll_offset + (digits_segments - k);
                if ( z >= 0 && z < buffer_chunk_size ) {
                  shift_buffer[0] = digits_buffer[buffer_offset + z];
                } else {
                  shift_buffer[0] = ' ';
                }
                shift_buffer[1] = '\0';
        
                // translate the buffer (for this many registers)
                shift_translate(digits_segment_registers);
                
                // push it to the register
                // false = we control the latch ourselves
                shift_send(3, false);
        
              }
                                      
              shift_latch(true); 
      
              // if we are scrolling
              if ( buffer_chunk_size > digits_segments ) {
  
                // delay(325);
                // make it scroll quicker if the message is longer...
                delay(325 - round((buffer_chunk_size / 4) * 3));
  
                // move over one
                buffer_scroll_offset++;
  
                // if we have finished the scroll
                if ( buffer_scroll_offset > buffer_chunk_size + digits_segments ) {
                  busy = false;
                }
              
              // otherwise it's non scrolling, wait for a bit before the next cycle
              } else {
  
                delay(2000);
  
                // i know this technically doesn't scroll, but it allows us to use the same code to move to the next chunk
                buffer_scroll_offset = 127;
  
              }
  
        
            }
          
          ///////////////////////////////////////
          // end of led shiftypants bit
          ///////////////////////////////////////
  
      } else {

        // something went wrong with the web bit...
        delay(1000);
        delay(1000);
        delay(1000);
        delay(1000);
        delay(1000);
      
      }

    } else {

      // something went wrong with the bios...
      delay(1000);
      delay(1000);
      delay(1000);
      delay(1000);
      delay(1000);
      
    }

  }

}
