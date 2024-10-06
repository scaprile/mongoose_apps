# Picowoose

The Raspberry Pi Pico-W meets Mongoose; here's the picodriver explained, with a very small MQTT example.

This example application describes the way to adapt the George Robotics CYW43 driver, present in the Pico-SDK, to work with Cesanta's [Mongoose](http://github.com/cesanta/mongoose/). We are then able to use Mongoose internal TCP/IP stack (with TLS 1.3), instead of lwIP (and MbedTLS). Future improvements will let us run this along MycroPython... "Pycowoose"... would you be interested ?
The driver code is taken as is from Cesanta's Github repository.
The example code is based on their MQTT client examples.

## Build

Building requires having [ARM GCC](https://launchpad.net/gcc-arm-embedded) and [CMake](https://cmake.org). This [tutorial](https://mongoose.ws/documentation/tutorials/tools/) describes how to install these for your OS.

The Makefile will clone Pico-SDK 1.5.1 from Github and start the build process; it will need some tweaks for Windows users, as `rm` is extensively used and I'm too lazy to support what I don't use, shame on me.

- Build with your Wi-Fi credentials
  ```sh
  $ make WIFI_SSID=mySSID WIFI_PASS=mypassword
  ```
- Plug your board; boot your Pico in USB Mass Storage Mode by holding down the BOOTSEL button while you plug it into the USB port. Once it is connected, release that button; it should mount as a new USB disk in your computer. Now either manually copy or drag and drop the uf2 file we just built (it lies inside the _build_ directory), into this new USB disk device.

- The Pico will flash this code and reboot, unmounting this disk and running the flashed code.

## Read about it

https://www.embeddedrelated.com/showarticle/1698.php

## My blog

https://www.scaprile.com/category/linge/
https://www.embeddedrelated.com/blogs-1/nf/Sergio_R_Caprile.php
