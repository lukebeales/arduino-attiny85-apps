// posts gps settings up to the internet
/*
  $GPGGA,120145.00,2654.77346,S,15256.66302,E,1,06,1.13,39.1,M,39.9,M,,*7F
  $GPGSA,A,3,01,26,31,04,03,16,,,,,,,4.28,1.13,4.13*0E
  $GPGSV,3,1,10,01,41,304,11,03,57,211,16,04,35,237,11,09,07,257,10*76
  $GPGSV,3,2,10,16,32,037,21,21,26,337,18,22,85,208,,26,42,083,24*79
  $GPGSV,3,3,10,31,33,141,21,32,05,099,19*71
  $GPGLL,2654.77346,S,15256.66302,E,120145.00,A,A*77
  $GPRMC,120146.00,A,2654.77311,S,15256.66315,E,0.805,,231221,,,A*65
  there should be a $GPVTG in here for speed and such
*/

char gps_latitude[12]; // GPGGA[2], GPGGA[3]
char gps_longitude[13]; // GPGGA[4], GPGGA[5]
byte gps_quality; // GPGGA[6]
char gps_altitude[8]; // GPGGA[9]
char gps_direction[4]; // GPVTG[1]
char gps_speed[4]; // GPVTG[7]
char gps_buffer[13];
byte gps_position_good_to_go = 0;
byte gps_direction_good_to_go = 0;

// 1 = position (GGA)
// 2 = direction (VTG)
void gps_grab_values(byte which_line_are_we) {

  if ( strlen(global_buffer) > 0 ) {

    if ( which_line_are_we == 1 ) {
      // reset the counter
      gps_position_good_to_go = 0;
    } else {
      // reset the counter
      gps_direction_good_to_go = 0;
    }

    byte segment = 0;
    byte segment_offset = 0;
    for ( byte i = 0; i < strlen(global_buffer); i++ ) {
      
      if ( global_buffer[i] == ',' ) {

        // terminate the buffer so it's all kosher
        gps_buffer[segment_offset] = '\0';

        // if there's actually something in the buffer
        if ( strlen(gps_buffer) > 0 ) {

          if ( which_line_are_we == 1 ) {
            if ( gps_quality == 0 ) {
              if ( segment == 5 ) { // quality
                gps_quality = gps_buffer[0] - 48;
              }
            } else {
              if ( segment == 1 ) { // latitude
                strcpy(gps_latitude, gps_buffer);
                gps_position_good_to_go++;
              } else if ( segment == 2 ) {  // latitude direction
                byte len = strlen(gps_latitude);
                gps_latitude[len] = gps_buffer[0];
                gps_latitude[len+1] = '\0';
                gps_position_good_to_go++;
              } else if ( segment == 3 ) { // longitude
                strcpy(gps_longitude, gps_buffer);
                gps_position_good_to_go++;
              } else if ( segment == 4 ) {  // longitude direction
                byte len = strlen(gps_longitude);
                gps_longitude[len] = gps_buffer[0];
                gps_longitude[len+1] = '\0';
                gps_position_good_to_go++;
              } else if ( segment == 8 ) { // altitude
                strcpy(gps_altitude, gps_buffer);
                gps_position_good_to_go++;
              }
            }
          } else if ( which_line_are_we == 2 ) {
            if ( gps_quality != 0 ) {
              if ( segment == 1 ) { // direction
                strcpy(gps_direction, gps_buffer);
                gps_direction_good_to_go++;
              } else if ( segment == 6 ) { // speed
                strcpy(gps_speed, gps_buffer);
                gps_direction_good_to_go++;
              }
            }
          }

        }

        // clear the buffer out
        gps_buffer[0] = '\0';

        // go to the next segment
        segment++;
        segment_offset = 0;
      } else {
        // update the buffer
        gps_buffer[segment_offset] = global_buffer[i];
        segment_offset++;
      }

    }

  }
}


void core_init() {

  pinMode(0, INPUT);
  pinMode(2, OUTPUT);

  SoftwareSerial gps(0, 2); // RX, TX


  gps.begin(9600);
  delay(200);


  // set switched on to turn on the wifi & GPS chip
  // bios_switch(true);

  // reset everything
  gps_latitude[0] = '\0';
  gps_longitude[0] = '\0';
  gps_altitude[0] = '\0';
  gps_direction[0] = '\0';
  gps_speed[0] = '\0';
  gps_quality = 0;

  serial_bookend[0] = '$';
  serial_bookend[1] = 'G';
  serial_bookend[2] = 'P';

  // start with the location line
  serial_bookend[3] = 'G';
  serial_bookend[4] = 'G';
  serial_bookend[5] = 'A';
  serial_bookend[6] = ',';
  serial_bookend[7] = '\0';

  gps.listen();

  gps_position_good_to_go = 0;
  gps_direction_good_to_go = 0;

  // find the location...
    while ( gps_position_good_to_go < 5 ) {
  
      serial_read(gps);
     
      // grab data
      gps_grab_values(1);
  
    }
  //
  
  // now see if we can find the speed and such within a few seconds
    serial_bookend[3] = 'V';
    serial_bookend[4] = 'T';
    serial_bookend[5] = 'G';

    // unsigned long time_we_started_looking = millis();
    unsigned long time_to_stop_looking = millis() + 3000;
    while (
      ( gps_direction_good_to_go < 2 ) &&
      ( time_to_stop_looking > millis() )
    ) {
  
      serial_read(gps);
     
      // grab data
      gps_grab_values(2);
  
    }
  //

  // load the wifi
  if ( web_open() ) {

    contentLength += web_data_chunk("latitude", NULL, gps_latitude, NULL, NULL, true);
    contentLength += web_data_chunk("longitude", NULL, gps_longitude, NULL, NULL, true);
    contentLength += web_data_chunk("altitude", NULL, gps_altitude, NULL, NULL, true);
    if ( gps_direction_good_to_go == 2 ) {
      contentLength += web_data_chunk("direction", NULL, gps_direction, NULL, NULL, true);
      contentLength += web_data_chunk("speed", NULL, gps_speed, NULL, NULL, true);
    }
    contentLength += web_data_end_all(true);

    // send the headers
    web_page_open(contentLength);

      // this is needed to make sure the wifi is ready for the data
      at_sleep(1000);
  
      // now send the data through
      web_data_chunk("latitude", NULL, gps_latitude, NULL, NULL, false);
      web_data_chunk("longitude", NULL, gps_longitude, NULL, NULL, false);
      web_data_chunk("altitude", NULL, gps_altitude, NULL, NULL, false);
      if ( gps_direction_good_to_go == 2 ) {
        web_data_chunk("direction", NULL, gps_direction, NULL, NULL, false);
        web_data_chunk("speed", NULL, gps_speed, NULL, NULL, false);
      }
      web_data_end_all(false);

      // this is to make sure all the data is sent correctly.
      at_sleep(1000);
          
    // close the webpage request
    web_page_close();

    web_close();
    at_sleep(1000);
    at_sleep(500);

  }
  
  // power down everything
  bios_sleep();
    
}
