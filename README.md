# Verbal Supplement
This project is a relatively simple combination of Bluetooth LE serial UART and scrolling textbuffer.  It's intended to be used in combination with the companion Flutter app which will listen for spoken words, translate them to text, then send them to the M5StickC.  The M5Stick is mounted in a harness that holds it in front of your facemask, so the overall effect is getting your own speech captioned for others to read.

## Under the hood
Verbal Supplement implements 2 BLE services:  UART for serial communication, and battery-level just for convenience.  On startup, these services are created, callbacks are attached, and the device begins advertising.  On connection from companion device, the screen will turn on and messages sent across the BLE serial interface will be written to the screen.  Newer messages will be written to the bottom of the display and older messages will scroll off the top.

On BLE disconnect, the screen will turn off and the device will go to sleep.  It can be woken up by pressing the main M5 button.

Any BLE device can be used to connect to the serial service, but it's intended to be used by the Flutter Verbal Supplement app.

## Other parts
This project is just one part of the full Verbal Supplement system.  Other parts include:
* 3D printed face-mounting case for M5StickC
* Flutter phone app for speech-to-text and communication to M5StickC
* (If on Android) Tasker task to automatically start Verbal Supplement app when registered M5StickC is nearby.
* a headset/mic of your choice so that only your own voice is picked up to be transcribed.

## Known Issues
* Battery life: M5StickC is rather anemic when it comes to battery life.  Don't expect to get more than novelty use out of this.  Still, good for trips to the grocery, take-out or other short-duration tasks.
* Screen size: Yeah, it's tiny.  To be readable at the recommended 6ft distance, the font size allows only 2 short rows on the display.  Who knows if it's worth it.