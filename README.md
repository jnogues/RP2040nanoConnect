# RP2040nanoConnect
Examples whit Arduino nano RP2040 connect.

Here I present a template to program the "Arduino nano RP2040 connect". As there are very few examples still for this board, I hope they can serve others and I look forward to suggestions for improvement !.
It is a program that uses various properties of the "mbed OS" such as Threads, Ticker and Whatcdog.
It has been programmed with the Arduino IDE 1.8.15, using the wifinina, PubSubClient and ArduinoJson libraries.

Apart from setup () and loop (), three more tasks have been created, th1, th2 and th3. The th1 is responsible for blinking the led13 and resetting the watchdog. The th2 manages the connection to the router and the mqtt broker. The th3 resets the board every hour. A tck1 Ticker with an irrelevant function has been added with the intention of seeing if it worked.

The intention is to create a solid program like a rock.

![nano](https://store-cdn.arduino.cc/uni/catalog/product/cache/1/image/520x330/604a3538c15e081937dbfbd20aa60aad/a/b/abx00053_00.default.jpg)

