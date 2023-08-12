// sends a random number to the web.
int test_random = 0;

void core_init() {

  // load the wifi
  web_open();

    test_random = random(1,9999);

    contentLength += web_data_chunk("random", test_random, NULL, NULL, NULL, true);
    contentLength += web_data_end_all(true);

    // send the headers
    web_page_open(contentLength);
  
    // now send the data through
    web_data_chunk("random", test_random, NULL, NULL, NULL, false);
    web_data_end_all(false);
          
    // close the webpage request
    web_page_close();

  // close the wifi
  web_close();

  delay(1500);  // allow things to finish

  bios_sleep();
  
}
