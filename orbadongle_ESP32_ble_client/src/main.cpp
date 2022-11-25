

#include <Arduino.h>
#include <ardumidi.h>

#include <BLEMidi.h>


#define ONBOARD_LED 2 // regular wroom32 led to show connection state, can be another pin for other devboards.   // #define ONBOARD_LED  5  // liligoboard
#define DEVICE_TO_CONNECT "Artiphon Orba 2"

int blink_counter = 0;
bool text_debug = false;  // instead of sending binary stuff to Serial2 port, send text description.

void connected();

void blink() // temporary disable led on each event, to indicate activity.
{
    blink_counter = 32000;  // number of loops before enabling led back.
    digitalWrite(ONBOARD_LED, 0);
}

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial2.printf("Note on : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
    }
    else
    {
        midi_note_on(channel, note, velocity);
    }

    blink();
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial2.printf("Note off : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
    }
    else
    {
        midi_note_off(channel, note, velocity);
    }

    blink();
}

void onAfterTouchPoly(uint8_t channel, uint8_t note, uint8_t pressure, uint16_t timestamp)
{

    if (text_debug)
    {

        Serial2.printf("Polyphonic after touch : channel %d, note %d, pressure %d (timestamp %dms)\n", channel, note, pressure, timestamp);
    }
    else
    {
        midi_key_pressure(channel, note, pressure);
    }

    blink();
}

void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial2.printf("Control change : channel %d, controller %d, value %d (timestamp %dms)\n", channel, controller, value, timestamp);
    }
    else
    {
        midi_controller_change(channel, controller, value);
    }

    blink();
}

void onProgramChange(uint8_t channel, uint8_t program, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial2.printf("Program change : channel %d, program %d (timestamp %dms)\n", channel, program, timestamp);
    }
    else
    {
        //  midi_program_change(byte channel, byte program);   // disabled as  I do not want to send it to MDX
    }

    blink();
}

void onAfterTouch(uint8_t channel, uint8_t pressure, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial2.printf("After touch : channel %d, pressure %d (timestamp %dms)\n", channel, pressure, timestamp);
    }
    else
    {

        midi_channel_pressure(channel, pressure); // have no idea if it works
    }

    blink();
}

void onPitchbend(uint8_t channel, uint16_t value, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial2.printf("Pitch bend : channel %d, value %d (timestamp %dms)\n", channel, value, timestamp);
    }
    else
    {
        midi_pitch_bend(channel, value);
    }

    blink();
}

void connected()
{
    if (text_debug)
      Serial2.println("Connected client");
}

void setup()
{
    pinMode(ONBOARD_LED, OUTPUT);
    // Serial2.begin(31250);
    Serial.begin(115200);
    Serial2.begin(31250, SERIAL_8N1, 16, 17);

    BLEMidiClient.begin("MIDI device");
    BLEMidiClient.setOnConnectCallback(connected);

    BLEMidiClient.setOnDisconnectCallback([]() { // To show how to make a callback with a lambda function
        if (text_debug)
            Serial2.println("Disconnected from srv");
    });

    // what to do when we get midi stuff from BLE

    BLEMidiClient.setNoteOnCallback(onNoteOn);
    BLEMidiClient.setNoteOffCallback(onNoteOff);
    BLEMidiClient.setAfterTouchPolyCallback(onAfterTouchPoly); // does not seem to be sent by orba anyway, but whatever.
    BLEMidiClient.setControlChangeCallback(onControlChange);
    //  BLEMidiClient.setProgramChangeCallback(onProgramChange);  // I do not want orba to sent program changes for my particular case
    BLEMidiClient.setAfterTouchCallback(onAfterTouch);
    BLEMidiClient.setPitchBendCallback(onPitchbend);

    if (text_debug)
        Serial2.println("Initializing bluetooth");
    BLEMidiClient.begin("ESP32MIDICLIENT"); //   the name you want to give to the ESP32

    // BLEMidiClient.enableDebugging();  // Uncomment to see debugging messages from the library
    // delay(2000);  // useless delay. Safety measure to be able read all the debug stuff from Serial2 console.
}

void loop()
{

    /// Serial2.println("loop");

    if (BLEMidiClient.isConnected())
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
    else
    {
        digitalWrite(ONBOARD_LED, 0);
    }

    if (!BLEMidiClient.isConnected())
    {
        // If we are not already connected, we try te connect to the first BLE Midi device we find
        int nDevices = BLEMidiClient.scan();
        if (text_debug)
            Serial2.print("num bluetooth devices ");
        if (text_debug)
            Serial2.println(nDevices);
        if (nDevices > 0)
        {
            // Searching for the one specific device we want to connect to ("Artiphon Orba 2")

            for (int i = 0; i < nDevices; i++)
            {
                if (text_debug)
                    Serial2.println(BLEMidiClient.getScannedDevice(i)->getName().c_str());

                if (strcmp(BLEMidiClient.getScannedDevice(i)->getName().c_str(), DEVICE_TO_CONNECT) == 0)
                {
                    if (text_debug)
                     Serial2.println("That's the one, let's try connecting");

                    if (BLEMidiClient.connect(0))
                    {
                        if (text_debug)
                            Serial2.println("Connection established");
                    }
                    else
                    {
                        if (text_debug)
                            Serial2.println("Connection failed");
                    }
                }
            }
            delay(3000); // We wait 3s before attempting a new connection
        }
    }
}
