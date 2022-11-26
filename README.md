# OrbaDongle


This is an DIY  implementation of  MIDI BLE  USB dongle  (similar to M-Vave MS-1 or CME WIDI devices).
It is created solely for my purpose of connecting Artiphon Orba 2 as a BLE MIDI controller to control linux-based Mod Duo X guitar processor/synth.

Being a MIDI class compliant thing, it  works with on Windows as well, and probably would work even with pc linux, mac and phones. But I have not tried.

It consists of two different controllers working together:

1) ESP32 Wroom devkit, that is capable to connect to Orba 2 as BLE client and retrieve MIDI data from Orba 2. (but uncapable of USB MIDI class compilant device emulation)
2) STM32 stm32f103cbt6 bluepill devboard  (this one has no BLE, but instead can emulate USB MIDI device)



ESP32 fetches midi data from Orba 2 via BLE using https://github.com/max22-/ESP32-BLE-MIDI library and streams data to STM32 via UART using https://github.com/FortySevenEffects/arduino_midi_library/ library.


STM32 receives data via UART using same arduino_midi_library, and forwards it to Mod Duo X (or PC)  via USB using  https://github.com/arpruss/USBComposite_stm32f1 library.


All the mentioned things are chained as below:



Orba2 -> (ESP32-BLE-MIDI) -> ESP32 -> (arduino_midi_library) -> STM32 -> (USBComposite_stm32f1 ) ->  USB (PC, or MDX)



Wiring:

ESP32 TX2 connected to A3 of STM32    // this one  sends data originated from Orba2  from ESP32 to STM32
ESP32 RX2 connected to A2 of STM32    // not used yet, but that's could be later used for flow of data into opposite direction.
ESP32 GND connected to GND of STM32   // ground

!!!Warning, achtung, внимание!!! 
	3.3V of STM32 connected to ESP32 3.3v   // This will power ESP32 from STM32 but *please*, do not power ESP32 with USB cable when this wire is connected, 
	to evade magic smoke leaving it.

	I would suggest uploading firmwares before adding this last wiring connection. And if you will need change to firwmare, disconnect this wiring.  

	Powering ESP32 from two sources in the same time might most probably fry it. Or something else. Do not do it.  
	
!!!Warning, achtung, внимание!!! 


Q&A:

Q: There are four data conversions, how bad is the input lag?

A: Still testing it on my MDX and playing with baud rates. At this point it is noticable when playing drum synths, but tolerable. 
For non-drum synths it feels ok.

However "tolerable" is subjective, so do not blame me if you will spend time building this project to find out that you hate the lag.
Lag does exist, you are warned.


Q: How to build all this stuff ?

A: This stuff is two separate Platform.IO projects (Platformio is an extension for VSCode). Probably you will need to undrestand how to use Platformio.
   STM32 is set to be flashed using STLINK, if you want to upload firmware other way, you will have to modify platformio.ini file.

Q: Will it work with Orba 1 ?

A: I have no idea. It probably could - they are quite similar after all, and I've tried to guess device as "Artiphon Orba"  
and added this name to filter, but I do not have Orba  1 to test.

Q: Will it work as generic poor man's WIDI replacement with other MIDI devices?

A: With some customization, I guess. 

   First of all, there are two filters DEVICE_TO_CONNECT1 DEVICE_TO_CONNECT2 defined in main.cpp of orbadongle_ESP32_ble_client.
   By default, ESP32 will ignore any BLE midi devices that are not Orba, and determines if it is Orba by these filters, seatching for "Artiphon Orba"  and "Artiphon Orba 2" .
   (I have other MIDI ble stuff, and had written the code to be sure that ESP32 will connect only to Orba.)
   
   If you want something else - you can modify the filter to match your device name, or set  bool connect_to_any = true  nearby and ESP32 will connect to the first BLE MIDI devices it sees.
   
    Also, this DIY solution works only as BLE Client.  If you need to connect to device that is not BLE Server, you can rewrite some code and  ESP32 will become a BLE Server and listen for connection (does not work with Orba2 by the way), as ESP32 BLE Library is able of being a server as well.
	
	Not all midi messages forwarding are implemented. (more on that below)
	
	Also, communication is  currently one direction currently, from BLE towards USB.
	

Q: Can I send midi back to Orba2? 

A: It is possible to write such code, but it is not yet implemented. Communication is one directional at the moment. I do not have reasons to implement opposite direction right now, but maybe later.
   
   
Q: What messages forwarding is implemented?

A: NoteOn, NoteOff, Pitchbend, ControlChange, ChannelAftertouch.  
   Also ProgramChange is possible and implemented, but is commented out, as it does not meet my personal use case.
   
   This was written for handling Orba2, Orba2 does not seem to send anything else, so I was lazy and skipped implementing any other messages to be forwared.
   
   If you  are so desperate that yout are borrowing this ugly code to implement your own midi ble to usb convertor for other use case,    
    be notified that there are other handlers in  ESP32 BLE MIDI library, STM32 f103 USBCOMPOSITE library and arduino_midi_libraries you can use and add.
	
    Some other messages could be implemented,  and some could not -  as there are 3 midi   underlying libraries involved, and if you need to stream a message type, it should be supported by all three libs.

    https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks

    https://github.com/arpruss/USBComposite_stm32f1/blob/master/USBMIDI.h

    https://github.com/max22-/ESP32-BLE-MIDI/blob/master/examples/04-Dump-Messages/04-Dump-Messages.ino




Additional taughts:


I am still testing the solution, and it seems to work more or less.

But I am concerned with following things:


1) Orba2 provides very small pitchbend range, probably will have to add some amplification of the pitchbend later. Orba2 connected by USB directly does the same, BTW.

2) I can feel some input lag on the periphery of my mind, especially for drums. 

3)  There is no error correction in transferring data between stm32 and esp32, so theoretically it can lead to noteOns without closing noteOffs, 
 and I am still not sure about if  I set baud rate for communication between controllers properly.

4) Solution is not tested enough yet

5) Also it involves programming two different microcontrollers, and it is unlikely that people who are not already into embedded programming are going to benefit benefit from this.

 