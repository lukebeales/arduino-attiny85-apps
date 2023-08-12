// this is a makeshift (HA!) shift register library for 74hc595 chips

// common pin
byte shift_latchPin = 2;

// this uses the 1 wire shift technique from here: https://www.romanblack.com/shift1.htm
// clock to clock, 10k resistor from clock to data (this is because arduino digitalwrite is slow), then 2n2 capacitor from data to ground
// 0 bit = 15uS low, 30uS high
// 1 bit = 1uS low, 15uS high
byte shift_clockAndDataPin = 4;

// this is for the typical 3 pin shiftiness
byte shift_clockPin = 0;
byte shift_dataPin = 4;

// we use normal characters which are `a..o to control 4 leds in any sequence, as we're using the lower 4 bits only
// it's wasteful at the cost of convenience of being able to enter normal characters from the serial port :)
// the ordering is pin 0 = the largest bit, pin 7 = the smallest bit
char shift_buffer[5]; // the two/four bytes to send to the shift register, plus a terminator

/*
  // the led patterns needed to show digits for a 7 segment display. clockwise starting at top, then the middle segment last
  const PROGMEM byte shift_7segment_alphanumeric[] = {
    0,    // space
    2,    // -
    252,  // 0
    96,
    218,
    242,
    102,
    182,
    190,
    224,
    254,
    246   // 9
  };
*/

// the led patterns needed to things for a 16 segment display. clockwise starting at top left, then the middle, then clockwise for the inside bits starting at top left
// stored as " -0..9A..Za..z".  Yep there's a space first as a fallback
const PROGMEM byte shift_16segment_alphanumeric[] = {
  0, 0,     // space
  5, 0,  // !
  32, 16,  // "
  60, 210,  // #
  221, 210, // $
  153, 219, // %
  142, 180,      // &
  0, 16,   // '
  72, 18,  // (
  132, 18, // )
  0, 255,  // *
  0, 210,  // +
  0, 129,  // ,
  0, 192,  // -
  4, 0,    // .
  0, 9,    // forwardslash
  255, 9,   // 0
  48, 8,
  238, 192,
  252, 64,
  49, 192,
  221, 192,
  223, 192,
  192, 10,
  255, 192,
  253, 192, // 9
  4, 128,  // :
  0, 17, // ;
  0, 12,   // <
  12, 192, // =
  0, 65,   // >
  224, 66,  // ?
  239, 80, // @
  243, 192, // A
  252, 82,
  207, 0,
  252, 18,
  207, 128,
  195, 128, // F
  223, 64,
  51, 192,
  204, 18,
  62, 0,
  3, 140,
  15, 0, // L
  51, 40,
  51, 36,
  255, 0, // O
  227, 192,
  255, 4,
  227, 196,
  221, 192,
  192, 18,  // T
  63, 0,
  3, 9,
  51, 5, // W
  0, 45,
  33, 194,
  204, 9, // Z
  72, 18,  // [
  0, 36,    // backslash
  132, 18, // ]
  32, 8,   // ^   
  12, 0,   // _
  0, 32,  // `
  14, 130, // a
  7, 130,
  6, 128,
  56, 66,
  6, 129,  // e
  64, 210,
  133, 146,
  3, 130,
  0, 2, // i
  6, 18,
  0, 30,
  3, 0,
  18, 194,  // m
  2, 130,
  6, 130,
  131, 144,
  129, 146, // q
  2, 128,
  8, 68,
  7, 128,
  6, 2,   // u
  2, 1,
  18, 5,
  0, 45,
  56, 80,
  4, 129,  // z
  72, 146, // {
  0, 18,   // |
  132, 82 // }
};


// somebody set us up the pins
void shift_init(byte pins) {
 
  // set up the pins for the shift register
  pinMode(shift_latchPin, OUTPUT);

  if ( pins == 2 ) {
    pinMode(shift_clockAndDataPin, OUTPUT);
  } else {
    pinMode(shift_clockPin, OUTPUT);
    pinMode(shift_dataPin, OUTPUT);
  }
}


// false while transmitting, true when done
void shift_latch(boolean on) {
  if ( on ) {
    // return the latch pin high to signal chip that it
    // no longer needs to listen for information
    digitalWrite(shift_latchPin, HIGH);
  } else {
    // ground latchPin and hold low for as long as you are transmitting
    digitalWrite(shift_latchPin, LOW);
  }
}


// translate some of the buffers depending on the character...
void shift_translate(byte registers) {

  if ( (char)shift_buffer[0] == '~' ) {

    for ( byte m = 0; m < registers; m++ ) {
      // pick a random character
      shift_buffer[m] = random(33, 125);
    }
    
  }


  byte shift_offset = 0;
  if ( shift_buffer[0] >= 33 && shift_buffer[0] <= 125 ) {
      shift_offset = ((shift_buffer[0] - 33) + 1) * registers;
  }


  // now find out the bits!
  // this is confusing because the buffer represents just 4 bits, not the 8 bits for a whole shift register
  // this needs to get the bottom 4 bits
  // then the top 4 bits
  // then repeat for the number of registers
  for ( byte n = 1; n <= registers; n++ ) {

    byte array_offset = registers - n;

    byte all_bits = 0;
//      if ( registers == 1 ) {
//        all_bits = pgm_read_byte_near(shift_7segment_alphanumeric + (shift_offset + array_offset));
    if ( registers == 2 ) {
      all_bits = pgm_read_byte_near(shift_16segment_alphanumeric + (shift_offset + array_offset));
    }

    byte a = registers * 2;
    byte b = (n-1) * 2;
    shift_buffer[a - (b + 1)] = all_bits & 0x0F; // low bits
    shift_buffer[a - (b + 2)] = all_bits >> 4;   // high bits

    // a hacky fix for the empty string issue.
    // this moves all the 4 bit things in to characters such as `a..o range
    // that means the shift_send function won't see the string as empty!
    shift_buffer[a - (b + 1)] += 96;
    shift_buffer[a - (b + 2)] += 96;
  }

  
  // this should always be a terminator
  shift_buffer[4] = '\0';
  if ( registers == 1 ) {
    // this has to always end at 2
    shift_buffer[2] = '\0';
  }
}


// push the buffer to the shift register
// pins - 2 = the shift1 capacitor timing trick, 3 = standard shift reg config
// latch - whether we flick the latch on and off for each send automatically
void shift_send(byte pins, boolean latch) {

  // failsafe, because we use two characters per register so this should be 2 or more
  byte len = strlen(shift_buffer);
  if ( len > 1 ) {

    if ( latch ) {
      shift_latch(false);
    }
  
    // this loop is backwards, it's the number of double characters to send through because we're only using 4 bits of each
    for ( byte m = 1; m <= len; m++ ) {
  
      for ( byte n = 0; n < 4; n++ )  {
  
        boolean bitToSend = !!(shift_buffer[len - m] & (1 << n));

        // if we're using the shift1 capacitor timing trick to save a pin
        if ( pins == 2 ) {

          byte lowTime = 15;  // 15 // 12
          byte highTime = 30; // 30 // 26
          if ( bitToSend == 1 ) {
            lowTime = 0;  // 1
            highTime = 15;  // 15 // 12
          }
          // digitalwrite apparently takes about 3.4uS
          // so this 'if' is to try and reduce clock cycles
          if ( lowTime == 0 ) {
            digitalWrite(shift_clockAndDataPin, LOW);
            digitalWrite(shift_clockAndDataPin, HIGH);
          } else {
            digitalWrite(shift_clockAndDataPin, LOW);
            delayMicroseconds(lowTime);
            digitalWrite(shift_clockAndDataPin, HIGH);
          }
          delayMicroseconds(highTime);
  
          // generic delay for the next shift
          delayMicroseconds(50);

        // standard shift register bits here
        } else {

          digitalWrite(shift_clockPin, LOW);

          if ( bitToSend == 1 ) {
            digitalWrite(shift_dataPin, HIGH);
          } else {
            digitalWrite(shift_dataPin, LOW);
          }
          
          digitalWrite(shift_clockPin, HIGH);
        }

      }
      
    }
  
    if ( latch ) {
      shift_latch(true);
    }

  }
  
}
