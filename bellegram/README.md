# "Bellegram", a DIY doorbell that sends you a Telegram message

A nice button using the M5 STAMP PICO (if you get to be able to press that tiny button, using Cesanta's [Mongoose](http://github.com/cesanta/mongoose/) as an HTTPS client to POST to the Telegram Bot API and send a message.
The code is based on their HTTP client examples.

## Build

Building requires Docker, it uses the latest ESP-IDF image from Espressif.

- Create a bot and get its token:
  - https://core.telegram.org/bots#how-do-i-create-a-bot
- Add the bot to your conversations (`/start`)
- Get your chat id
  ```sh
  $ curl https://api.telegram.org/<your bot token>/getUpdates 
  ```
- Build with your data
  ```sh
  $ make WIFI_SSID=mySSID WIFI_PASS=mypassword BOT_TOKEN=mytoken CHAT_ID=123456789
  ```
- Plug your board and flash
  ```sh
  $ make flash
  ```

 If you don't have Docker, you should know how to build using your local copy of the IDF and tools, so just check the Makefile for how to put credentials into the EXTRA_CFLAGS environment variable.

## Read about it

## See it in action

https://youtu.be/2Xs14qVFyV4

## My blog
