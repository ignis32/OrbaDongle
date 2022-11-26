# OrbaDongle


This is an DIY  implementation of  MIDI BLE  USB dongle  (similar to M-Vave MS-1 or CME WIDI devices).
It is created solely for my purpose of connecting Artiphon Orba 2 as a BLE MIDI controller to control Mod Duo X guitar processor/synth.

It consists of two different controllers working together:

1) ESP32 Wroom devkit, that is capable to connect to Orba 2 as BLE client and retrieve MIDI data from Orba 2. (but uncapable of USB MIDI class compilant device emulation)
2) STM32 stm32f103cbt6 bluepill devboard  (this one has no BLE, but instead can emulate USB MIDI device)



ESP32 fetches midi data from Orba 2 via BLE using https://github.com/max22-/ESP32-BLE-MIDI library and streams data to STM32 via UART using https://github.com/FortySevenEffects/arduino_midi_library/ library.


STM32 receives data via UART using same arduino_midi_library, and forwards it to Mod Duo X (or PC)  via USB using  https://github.com/arpruss/USBComposite_stm32f1 library.



Wiring:

ESP32 TX2 connected to A3 of STM32    // this one  sends data originated from Orba2  from ESP32 to STM32
ESP32 RX2 connected to A2 of STM32    // not used yet, but that's for flow of data into opposite direction.
ESP32 GND connected to GND of STM32   // ground

!!!Warning, achtung, внимание!!! 
3.3V of STM32 connected to ESP32 3.3v   // This will power ESP32 from STM32 but *please*, do not power ESP32 with USB cable when this wire is connected. 

I would suggest uploading firmwares before adding this last wiring connection. And if you will need change to firwmare, disconnect this wiring.

Powering ESP32 from two sources in the same time will most probably fry it. Or something else. Do not do it. 
 


Additional information:

Q: Will it work with Orba 1 ?
A:  I have no idea.



 /*  Notice:
    
    This was written for handling Orba2. Orba2 does not seem to send PolyAftertouch (per key), transport signals or clock, 
    and I was too lazy to implement functions I'm not going to use.

    If you are borrowing this code to implement your own midi ble to usb convertor for other use case,    
    be notified that there are other handlers in  ESP32 USBCOMPOSITE and arduino_midi_libraries you can use.

    https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks

    https://github.com/arpruss/USBComposite_stm32f1/blob/master/USBMIDI.h

    https://github.com/max22-/ESP32-BLE-MIDI/blob/master/examples/04-Dump-Messages/04-Dump-Messages.ino


 */
