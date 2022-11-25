#include <Arduino.h>
#include <USBComposite.h>
USBMIDI usbmidi;
USBCompositeSerial CompositeSerial;


#include <MIDI.h>
//#HardwareSerial TxRxSerial(2);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, DIN_MIDI);

#define ONBOARD_LED PB2


bool text_debug = true;


int blink_counter = 0;
void blink() // temporary disable led on each event, to indicate activity.
{
    blink_counter = 32000; // number of loops before enabling led back.
    digitalWrite(ONBOARD_LED, LOW);
}

void unblink()
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



void onNoteOn(byte  channel, byte  note, byte  velocity)
{
    if (text_debug)
    {
        CompositeSerial.print ("Note on :");
        CompositeSerial.print (" "); CompositeSerial.print (channel);
        CompositeSerial.print (" "); CompositeSerial.print (note);
        CompositeSerial.print (" "); CompositeSerial.println ( velocity );
    }
     usbmidi.sendNoteOn(channel-1, note, velocity);

    blink();
}

void onNoteOff(byte  channel, byte  note, byte  velocity  )
{
    if (text_debug)
    {

        
        CompositeSerial.print ("Note off: ");
   
        CompositeSerial.print (" "); CompositeSerial.print (channel);
        CompositeSerial.print (" "); CompositeSerial.print (note);
        CompositeSerial.print (" "); CompositeSerial.println ( velocity );
        
    }
     usbmidi.sendNoteOff(channel-1, note, velocity);
    blink();
}






void init_usb_stuff()
{
    usbmidi.registerComponent();
    CompositeSerial.registerComponent();
    USBComposite.begin();
    // while (!USBComposite); // might hang on this moment if nobody connects. Should be disabled for prod build probably
    USBComposite.setProductId(0x0031);

    Serial2.begin(31250);


    DIN_MIDI.setHandleNoteOn(onNoteOn);
    DIN_MIDI.setHandleNoteOff(onNoteOff);
    DIN_MIDI.begin(MIDI_CHANNEL_OMNI);
}

void setup()
{
    init_usb_stuff();
    pinMode(PB2, OUTPUT);
    digitalWrite(PB2, HIGH);
}



void loop()
{

    //CompositeSerial.print(".");
    unblink();

     
if (DIN_MIDI.read())                    // If we have received a message
    {
        //CompositeSerial.println("MESSAGE");
        blink();


    }
       
}