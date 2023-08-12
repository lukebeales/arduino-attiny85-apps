// sets shift registers depending on a pattern
/*
 * this will read the bios settings, starting at A until it doesn't find one, and then loop.
 * they are in the format of repeat, timing, patterns (specified in blocks of 4 leds, by characters `abcd...o)
 * the lower 4 bits in each character represent 4 leds
 * ~ = pick a random character between ` and o
 * the first number is how many times to repeat
 * the second number is a delay * 25 * itself
 * 
 * two shift registers example
 * r = how many registers we have
 * A = 12h```d```b```a````h```d```b```a````h```d```b```a````h```d```b```a
 * B = 12```b```d```h``a```b```d```h``a```b```d```h``a```b```d```h```
 * C = 94~~~~
 * D = 35o````o````o````o``o``o``
 * E = 91~~~~
 * ...
 * Z = 83``abcb``b`o`h``
 * instructions MUST start at A (ascii 65) and finish at Z (ascii 90)
 *
 *
 * one shift register example
 * r = 1
 * A = 12`a`b`d`ha`b`d`h`d`b`a``h`d`b
*/


void core_init() {

  // allow things to happen
  at_sleep(1000);
  at_sleep(1000);
  at_sleep(1000);

  byte last_instruction = 0;

  // see if we have an instructions count
  bios_get('i');
  if ( strlen(global_buffer) > 0 ) {
    last_instruction = (byte)global_buffer[0] - 65;  // this starts at 1 for funsies
  }

  // see if we have a registers setting
  bios_get('r');
  if ( strlen(global_buffer) > 0 ) {

    // how many registers there are
    byte registers = (byte)global_buffer[0] - 48;  // this one starts at 1 just to make it difficult for us

    // this means we can only tie together 8 shift registers which should be enough for this tiny thing.
    if ( ( registers > 0 ) && ( registers <= 8 ) ) {

      // 'init' the shift register
      shift_init(3);
    
      while( true ) {
    
        byte i = 0;
        while ( i <= last_instruction ) {    // this is A..Z
//        while ( i + 65 <= 125 ) {   // this is A..z
    
          // look for an instruction for this code
          bios_get((char)(i + 65), true);
    
          // if an instruction exists    
          if (
            ( strlen(global_buffer) > 2 + (registers * 2) ) &&
            ( global_buffer[0] >= 48 ) &&
            ( global_buffer[0] <= 57 ) &&
            ( global_buffer[1] >= 48 ) &&
            ( global_buffer[1] <= 57 )
          ) {
    
            
            // this is the specific instruction loop
            for ( byte j = 0; j < ((byte)global_buffer[0] - 48); j++ ) {
    
              // this is the instructions sequence loop
              for ( byte k = 2; k < strlen(global_buffer); k += (registers * 2) ) {

                shift_latch(false);
                
                for ( byte m = 1; m <= registers; m++ ) {

                  // load the buffer up
                  shift_buffer[0] = global_buffer[(((k + (registers * 2)) - 1) - (m * 2)) + 1];
                  shift_buffer[1] = global_buffer[(((k + (registers * 2)) - 1) - (m * 2)) + 2];
                  shift_buffer[2] = '\0';

                  if ( shift_buffer[0] == '~' ) {
                    shift_buffer[0] = random(96, 112);
                  }
                  if ( shift_buffer[1] == '~' ) {
                    shift_buffer[1] = random(96, 112);
                  }

                  // push it to the register
                  shift_send(3, false);

                }
                                                    
                shift_latch(true); 
                       
                // delay keys start at 0
                delay((((byte)global_buffer[1] - 48) * 25) * ((byte)global_buffer[1] - 48));
              }
              
            }
            
            // move to the next instruction
            i++;
    
          } else {
    
            // reset the loop
            i = 0;
    
          }

        }

      }

    }

  }
  
}
