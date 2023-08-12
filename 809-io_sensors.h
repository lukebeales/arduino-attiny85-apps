byte pin0;  // magnet, tilt
unsigned int pinA1; // temperature
unsigned int pinA2; // light

void core_init() {

  // find the values of the pins
    pinMode(0, INPUT);
    pin0 = digitalRead(0);

      pinMode(A1, INPUT);
      unsigned int pinA1_samples[5];
      for ( byte i = 0; i < 5; i++ ) {
        pinA1_samples[i] = analogRead(A1);
        delay(10);
      }

      float pinA1_average;
      pinA1_average = 0;
      for (byte i = 0; i < 5; i++ ) {
        pinA1_average += pinA1_samples[i];
      }
      pinA1_average /= 5;

      // https://learn.adafruit.com/thermistor/using-a-thermistor
        // convert the value to resistance
        // this depends which way the thermistor works
        // pinA1_average = 1023 / pinA1_average - 1;    
        pinA1_average = 1023 / (1023 - pinA1_average) - 1;  // this was to fix it from working backwards

        pinA1_average = 10000 / pinA1_average;
  
        // now work out the actual celcius
        float steinhart;
        steinhart = pinA1_average / 10000;     // (R/Ro)
        steinhart = log(steinhart);                  // ln(R/Ro)
        steinhart /= 3977;                   // 1/B * ln(R/Ro)    // this could be 3950 or 3977, depends on the datasheet
        steinhart += 1.0 / (25 + 273.15); // + (1/To)
        steinhart = 1.0 / steinhart;                 // Invert
        steinhart -= 273.15;                         // convert absolute temp to C
  
        // now round it, because our web_data_chunk function doesn't support floats.
        pinA1 = (int)steinhart;
      //
      
      /* // https://create.arduino.cc/projecthub/Marcazzan_M/how-easy-is-it-to-use-a-thermistor-e39321
        float RT, VR, ln, TX, T0, VRT;
        VRT = pinA1_average;
        T0 = 25 + 273.15;
        VRT = (5.00 / 1023.00) * VRT;      //Conversion to voltage
        VR = 5.00 - VRT;                     // 5 = VCC
        RT = VRT / (VR / 10000);               //Resistance of RT
      
        ln = log(RT / 10000);
        TX = (1 / ((ln / 3977) + (1 / T0))); //Temperature from thermistor
      
        pinA1 = TX - 273.15; 
      */
            
    //

    pinMode(A2, INPUT);
    pinA2 = (100.00 / 1023) * analogRead(A2);  // make this a percentage

  //

  // load the wifi
  web_open();

    contentLength += web_data_chunk("int", pin0, NULL, NULL, NULL, true);
    contentLength += web_data_chunk("temp", pinA1, NULL, NULL, NULL, true);
    contentLength += web_data_chunk("light", pinA2, NULL, NULL, NULL, true);
    contentLength += web_data_end_all(true);

    // send the headers
    web_page_open(contentLength);

      at_sleep(125);
      
      // now send the data through
      contentLength += web_data_chunk("int", pin0, NULL, NULL, NULL, false);
      contentLength += web_data_chunk("temp", pinA1, NULL, NULL, NULL, false);
      contentLength += web_data_chunk("light", pinA2, NULL, NULL, NULL, false);
      web_data_end_all(false);
          
    // close the webpage request
    web_page_close();

  // close the wifi
  web_close();

  // delay(1500);  // allow things to finish
  at_sleep(1000);
  at_sleep(500);

  bios_sleep();

}
