// ******************************************************************************************
//
// OLED NEEDS TO BE SET TO 3.3V OTHERWISE ODD THINGS HAPPEN REGARDING THE SERIAL BUS
//
// ******************************************************************************************


/*
  https://github.com/datacute/Tiny4kOLED
  https://wokwi.com/arduino/projects/293574465813479944
*/

// http://www.technoblogy.com/show?23OS
// maybe try this alternative oled to save memory?


// https://github.com/SpenceKonde/ATTinyCore
// this says we need pullup resistors (10k) on the i2c pins
// it doesn't sem to need it once initialised but unsure about the start as now its not working for me.


////////////////////////////////////////////

  // Choose your I2C implementation before including Tiny4kOLED.h
  // The default is selected is Wire.h
  
  // To use the Wire library:
  //#include <Wire.h>
  
  // https://github.com/puuu/USIWire
  // #include <USIWire.h>
  
  // To use the TinyI2C library from https://github.com/technoblogy/tiny-i2c
  // *** this has low ram usage due to no buffers
  // *** this also needs https://github.com/SpenceKonde/ATTinyCore
  //#include <TinyI2CMaster.h>
  
  // To use the Adafruit's TinyWireM library:
  // *** REMEMBER, IN DOCUMENTS/ARDUINO/LIBRARIES/TINYWIREM/TINYWIREM.H THIS CAN BE CHANGED TO FREE UP SOME PRECIOUS RAM
  ////#define USI_BUF_SIZE 18 //!< bytes in message buffer
  //#define USI_BUF_SIZE 8 //!< bytes in message buffer
  #include <TinyWireM.h>
  
  // The blue OLED screen requires a long initialization on power on.
  // The code to wait for it to be ready uses 20 bytes of program storage space
  // If you are using a white OLED, this can be reclaimed by uncommenting
  // the following line (before including Tiny4kOLED.h):
  #define TINY4KOLED_QUICK_BEGIN
  
  #include <Tiny4kOLED.h>

/////////////////////////////////////////////////////////////
// define our custom pixel font here...
// we're doing this to take advantage of the oled print functions

const uint8_t ssd1306xled_fontPixels [] PROGMEM = {
  0x00,0x00,0x00,0x00, // 48 0 empty
  0xff,0xff,0xff,0xff  // 49 1 █
};

const DCfont TinyOLEDFontPixels = {
  (uint8_t *)ssd1306xled_fontPixels,
  4, // character width in pixels
  1, // character height in pages (8 pixels)
  48,49 // ASCII extents
  };
#define FONTPIXELS (&TinyOLEDFontPixels)


// now outline how each digit looks
// based off this font: https://www.ffonts.net/Modern-DOS-8x8.font
const PROGMEM byte oled_pixel_digits[] = {
/* these are just test patterns
  255,255,255,255,255,255,255,255,
  255,0,255,0,255,0,255,0,
  255,129,129,129,129,129,129,255,
  170,85,170,85,170,85,170,85,
*/
  124,206,206,222,246,230,230,124,  // 0
   24, 56,120, 24, 24, 24, 24,126,  // 1
  124,198,  6,  6, 28,112,198,254,  // 2
  124,198,  6, 60,  6,  6,198,124,  // 3
   12, 28, 60,108,204,254, 12, 30,  // 4
  254,192,192,252,  6,  6,198,124,  // 5
  124,198,192,252,198,198,198,124,  // 6
  254,198,  6, 12, 24, 48, 48, 48,  // 7
  124,198,198,124,198,198,198,124,  // 8
  124,198,198,198,126,  6,198,124,  // 9
   60,102,  6, 12, 24, 24, 0, 24,  // ?
   24, 60, 60, 60, 24, 24, 0, 24   // !
};



/////////////////////////////////////////////////////////////

void oled_init() {
  // Send the initialization sequence to the oled. This leaves the display turned off
  oled.begin(128, 64, sizeof(tiny4koled_init_128x64br), tiny4koled_init_128x64br);
  // without parameters it sets it to 128x32
  // oled.begin();

  // Two rotations are supported,
  // The begin() method sets the rotation to 1.
  //oled.setRotation(0);

  // Some newer devices do not contain an external current reference.
  // Older devices may also support using the internal curret reference,
  // which provides more consistent brightness across devices.
  // The internal current reference can be configured as either low current, or high current.
  // Using true as the parameter value choses the high current internal current reference,
  // resulting in a brighter display, and a more effective contrast setting.
  oled.setInternalIref(true);

  // Two fonts are supplied with this library, FONT8X16 and FONT6X8
  // Other fonts are available from the TinyOLED-Fonts library
  oled.setFont(FONTPIXELS);

  // Clear the memory before turning on the display
  oled.clear();

  // Turn on the display
  oled.on();

  // Switch the half of RAM that we are writing to, to be the half that is non currently displayed
//  oled.switchRenderFrame();

}
/*  
  void oled_open() {
    oled.clear();
  }
  
  void oled_close() {
    // Swap which half of RAM is being written to, and which half is being displayed.
    // This is equivalent to calling both switchRenderFrame and switchDisplayFrame.
    oled.switchFrame();
  }
*/

// this is super inefficient but it's the best I can do at this time of night
void oled_pixeldigit(byte X, byte Y, byte digit) {

  // go through each line of the digit data
  for ( byte yy = 0; yy < 8; yy++ ) {
    
    // i'm an idiot.  it took me hours to realise i need to read it from progmem rather than as a straight array
    byte line_to_read = pgm_read_byte_near(oled_pixel_digits + (digit * 8) + yy);

    // go through each bit for each line
    for ( byte xx = 0; xx < 8; xx++ )  {

      // move the cursor to the right spot
      oled.setCursor((X * 4) + (xx * 4), Y + yy);

      // get the entry that matches the digit offset and the correct line
      boolean bitToSend = bitRead(line_to_read, 7-xx);

      if ( bitToSend == 1 ) {

        // hope for the best!
        oled.print(1);

      }

    }

  } 

}

// get the digit at $digit, starting at the least bit.
// eg.  12345 with a digit specified of 0, will return 5
byte oled_get_digit_at(uint16_t number, byte digit) {
  byte remainder = 0;
  for ( byte i = 0; i <= digit; i++ ) {
    remainder = number % 10;
    number = floor(number / 10);
  }
  return remainder;
}

///////////////////////////////////////


byte oled_step = 0;
unsigned long oled_last_fetch_time = 0;   // this is to get it to refresh each minute with its own timer rather than the bios

void core_init() {

  // give everything a chance to happen
  at_sleep(1000);

  while (true) {

    // read the time from the bios...
    bios_time();

    delay(250);

    // if we have an invalid looking time, or enough time has passed, try and fetch it from the web
    if (
      ( at_timestamp == 0 ) ||
      ( millis() > oled_last_fetch_time + 300000 )

    ) {

      oled_init();
      delay(250); // this seems to make it play nice

      oled_last_fetch_time = millis();

      oled.clear();
      oled_pixeldigit(12, 0, 10);

      // let everything calm down
      delay(250);

      bios_switch(true);
    
      // load the wifi
      if ( web_open() ) {
  
        // send the headers
        web_page_open(contentLength);
      
          // aren't sending any data, so we don't need to end the data
          // web_data_end_all(false);
  
          web_get_content();
  
        // close the webpage request
        web_page_close();
  
        // close the wifi
        web_close();
  
      }
      
      bios_switch(false);
  
      // if we have no time still, lets give it 10 seconds to recover...
      if ( at_timestamp == 0 ) {
        oled.clear();
        oled_pixeldigit(12, 0, 11);

        delay(1000);
        delay(1000);
        delay(1000);
        delay(1000);
        delay(1000);
        delay(1000);
        delay(1000);
        delay(1000);
        delay(1000);
        delay(1000);
      }

    } else {
  
      oled.clear();
      uint16_t num = 0;   // this needs to be large to store the year.
      if ( oled_step == 0 ) {

        oled_init();
        delay(250); // this seems to make it play nice

        num = day(at_timestamp);
        oled_pixeldigit(0, 0, oled_get_digit_at(num, 1));
        oled_pixeldigit(8, 0, oled_get_digit_at(num, 0));
        num = month(at_timestamp);
        oled_pixeldigit(16, 0, oled_get_digit_at(num, 1));
        oled_pixeldigit(24, 0, oled_get_digit_at(num, 0));
      } else if ( oled_step == 1 ) {
        num = year(at_timestamp);
        oled_pixeldigit(0, 0, oled_get_digit_at(num, 3));
        oled_pixeldigit(8, 0, oled_get_digit_at(num, 2));
        oled_pixeldigit(16, 0, oled_get_digit_at(num, 1));
        oled_pixeldigit(24, 0, oled_get_digit_at(num, 0));
      } else if ( oled_step == 2 ) {
        num = hour(at_timestamp);
        oled_pixeldigit(0, 0, oled_get_digit_at(num, 1));
        oled_pixeldigit(8, 0, oled_get_digit_at(num, 0));
        num = minute(at_timestamp);
        oled_pixeldigit(16, 0, oled_get_digit_at(num, 1));
        oled_pixeldigit(24, 0, oled_get_digit_at(num, 0));
      } else {
        num = second(at_timestamp);
        oled_pixeldigit(16, 0, oled_get_digit_at(num, 1));
        oled_pixeldigit(24, 0, oled_get_digit_at(num, 0));
      }
      
      // } else if ( oled_step == 3 ) {
        // *** WHEN PRINTING A LONG LINE, THE DISPLAY AUTOMATICALLY WRAPS!
        // oled.setCursor(0, 0);
        // oled.print(global_buffer);

      // this is to make the seconds bit more accurate.
      delay(250);
      if ( oled_step < 3 ) {
        delay(1000);
        delay(1000);
      }
        
      oled_step++;
      if ( oled_step >= 8 ) {
        oled_step = 0;
      }

    }  

  }

}
