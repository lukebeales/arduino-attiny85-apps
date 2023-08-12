# arduino-attiny85-apps
A modular arduino sketch containing different apps for an attiny85 based 'resourcible' board

```
                +-----+
 reset     5 ----|     |---- vcc   3.3v, 4.3 (5v + diode), or 5v
    TX     3 ----|     |---- 2/A1  in/out/sck/int
in/out  4/A2 ----|     |---- 1     RX
         GND ----|     |---- 0     in/out/sda/pwm
                 +-----+


<800 = core files/libs
8xx = individual apps
999-init = choose which app to include and compile

// https://www.arduino.cc/en/Hacking/LibraryTutorial
```
