#include <Arduino.h>

#include <USBComposite.h>           // STM32 library to emulate USB midi device + Serial port
USBMIDI usbmidi;                    // to emulate USB midi device
USBCompositeSerial CompositeSerial; // plus serial port, (instead of Serial object)

#include <MIDI.h> // arduino_midi_library, to decode incoming data from ESP32

//#HardwareSerial TxRxSerial(2);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, UART_MIDI); //  RX on A3,  TX on A2

#define ONBOARD_LED PB2 // led to show commuincation activity
#define BAUD_RATE 115200

bool text_debug = false; // debug print every incoming midi event from UART to serial port.

int blink_counter = 0;
void blink() // temporary disable led on each event, to indicate activity.
{
    blink_counter = 10000; // number of loops before enabling led back.
    digitalWrite(ONBOARD_LED, LOW);
}

void unblink() // enable led back, but only after  loop counter comes to zero.
{
    if (blink_counter <= 0)
    {
        digitalWrite(ONBOARD_LED, 1);
    }
    else
    {
        blink_counter--;
    }
}

//////////////  Handler functions for midi events recieved by UART,  that do push the same events to USB

void onNoteOn(byte channel, byte note, byte velocity)
{
    if (text_debug)
    {
        CompositeSerial.print("Note on :");         // CompositeSerial does not understand printf, thats why so ugly.
        CompositeSerial.print(" ");
        CompositeSerial.print(channel);
        CompositeSerial.print(" ");
        CompositeSerial.print(note);
        CompositeSerial.print(" ");
        CompositeSerial.println(velocity);
    }
    usbmidi.sendNoteOn(channel - 1, note, velocity); // channel -1 :    USB composite counts channels from 0, arduino_midi_library from 1
    blink();
}

void onNoteOff(byte channel, byte note, byte velocity)
{
    if (text_debug)
    {
        CompositeSerial.print("Note off: ");
        CompositeSerial.print(" ");
        CompositeSerial.print(channel);
        CompositeSerial.print(" ");
        CompositeSerial.print(note);
        CompositeSerial.print(" ");
        CompositeSerial.println(velocity);
    }
    usbmidi.sendNoteOff(channel - 1, note, velocity);
    blink();
}

void onControlChange(byte channel, byte controller, byte value)
{
    if (text_debug)
    {
        CompositeSerial.print("CC: ");
        CompositeSerial.print(" ");
        CompositeSerial.print(channel);
        CompositeSerial.print(" ");
        CompositeSerial.print(controller);
        CompositeSerial.print(" ");
        CompositeSerial.println(value);
    }
    usbmidi.sendControlChange(channel - 1, controller, value);
    blink();
}

void onProgramChange(byte channel, byte number)
{
    if (text_debug)
    {
        CompositeSerial.print("Program change: ");
        CompositeSerial.print(" ");
        CompositeSerial.print(channel);
        CompositeSerial.print(" ");
        CompositeSerial.println(number);
    }
    usbmidi.sendProgramChange(channel - 1, number);
    blink();
}

void onPitchBend(byte channel, int bend)
{
    if (text_debug)
    {
        CompositeSerial.print("Pitchbend: ");
        CompositeSerial.print(" ");
        CompositeSerial.print(channel);
        CompositeSerial.print(" ");
        CompositeSerial.println(bend);
    }
    usbmidi.sendPitchChange(channel-1,  bend);
    blink();
}

void onAfterTouchChannel(byte channel, byte pressure)
{
    if (text_debug)
    {
        CompositeSerial.print("Aftertouch: ");
        CompositeSerial.print(" ");
        CompositeSerial.print(channel);
        CompositeSerial.print(" ");
        CompositeSerial.println(pressure);
    }
    usbmidi.sendAfterTouch(channel-1, pressure);
    blink();
}

void init_uart_midi()
{
    Serial2.begin(BAUD_RATE); //  . Not sure if arduino_midi_library gets is though. TBD: VERIFY BAUD RATE.

    // set handler functinos for incoming events
    UART_MIDI.setHandleNoteOn(onNoteOn);
    UART_MIDI.setHandleNoteOff(onNoteOff);

    UART_MIDI.setHandleControlChange(onControlChange);

    //  Orba 2 sends program changes. However, it does not fit my personal use case, therefore disabled.
    // UART_MIDI.setHandleProgramChange(onProgramChange);

    UART_MIDI.setHandlePitchBend(onPitchBend);
    UART_MIDI.setHandleAfterTouchChannel(onAfterTouchChannel);

    UART_MIDI.begin(MIDI_CHANNEL_OMNI);
}

void init_usb_stuff()
{
    usbmidi.registerComponent();
    CompositeSerial.registerComponent();
    USBComposite.begin();
    // while (!USBComposite); // might hang on this moment if nobody connects. Should be disabled for prod build probably
    USBComposite.setProductId(0x0031);
}

void init_onboard_led()
{
    pinMode(PB2, OUTPUT);
    digitalWrite(PB2, HIGH);
}

void setup()
{
    init_usb_stuff();
    init_uart_midi();
    init_onboard_led();
}

void loop()
{
    unblink();

    if (UART_MIDI.read()) // If we have received a message
    {
        blink();
    }
}