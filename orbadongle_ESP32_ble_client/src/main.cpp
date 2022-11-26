#include <Arduino.h>
#include <BLEMidi.h>  // ESP32 BLE MIDI library
#include <MIDI.h>  // fourtyseven arduino midi library

HardwareSerial TxRxSerial(2);
MIDI_CREATE_INSTANCE(HardwareSerial, TxRxSerial, UART_MIDI);

#define ONBOARD_LED 2 // regular wroom32 led to show connection state and activity, can be another pin for other devboards.   // #define ONBOARD_LED  5  // liligoboard
#define BAUD_RATE 115200
#define DEVICE_TO_CONNECT1 "Artiphon Orba 2" 
#define DEVICE_TO_CONNECT2 "Artiphon Orba" 
bool connect_to_any = false;     // ignore filtering and connect to the first ble midi device you see
bool text_debug = false;  // instead of sending binary stuff to Serial port, send text description.

void connected();


int blink_counter = 0;

void blink() // temporary disable led on each event, to indicate activity.
{
    blink_counter = 32000;  // number of loops before enabling led back. 
    digitalWrite(ONBOARD_LED, 0);
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

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial.printf("Note on : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
    }
   
    UART_MIDI.sendNoteOn(note, velocity, channel+1);  // ESP32 BLE library counts  channels from zero, arduino_midi_library counts from 1
   
    blink();
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial.printf("Note off : channel %d, note %d, velocity %d (timestamp %dms)\n", channel+1, note, velocity, timestamp);
    }
    
    UART_MIDI.sendNoteOff(note, velocity, channel+1);  
    blink();
}

void onAfterTouchPoly(uint8_t channel, uint8_t note, uint8_t pressure, uint16_t timestamp) // ignored  and not implemented on stm32 side, sorry
{
    if (text_debug)
    {
        Serial.printf("Polyphonic after touch : channel %d, note %d, pressure %d (timestamp %dms)\n", channel+1, note, pressure, timestamp);
    }
    UART_MIDI.sendAfterTouch(note, pressure, channel+1);

    blink();
}

void onAfterTouch(uint8_t channel, uint8_t pressure, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial.printf("After touch : channel %d, pressure %d (timestamp %dms)\n", channel+1, pressure, timestamp);
    }
    UART_MIDI.sendAfterTouch(pressure, channel+1);
    blink();
}

void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial.printf("Control change : channel %d, controller %d, value %d (timestamp %dms)\n", channel+1, controller, value, timestamp);
    }
    UART_MIDI.sendControlChange(controller, value, channel+1);
    blink();
}

void onProgramChange(uint8_t channel, uint8_t program, uint16_t timestamp)  // sent to STM32 but commented out there for personal reasons
{
    if (text_debug)
    {
        Serial.printf("Program change : channel %d, program %d (timestamp %dms)\n", channel+1, program, timestamp);
    }
    UART_MIDI.sendProgramChange(	program,  channel+1);	
    blink();
}



void onPitchbend(uint8_t channel, uint16_t value, uint16_t timestamp)
{
    if (text_debug)
    {
        Serial.printf("Pitch bend : channel %d, value %d (timestamp %dms)\n", channel+1, value, timestamp);
    }
    UART_MIDI.sendPitchBend ( value,channel+1);
    blink();
}

void connected()
{
    if (text_debug)
      Serial.println("Connected client");
}


void init_ble()
{
    BLEMidiClient.begin("MIDI device");
    BLEMidiClient.setOnConnectCallback(connected);
    BLEMidiClient.setOnDisconnectCallback([]() { // To show how to make a callback with a lambda function
        if (text_debug)
            Serial.println("Disconnected from srv");
        
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
        Serial.println("Initializing bluetooth");
    BLEMidiClient.begin("ESP32MIDICLIENT"); //   the name you want to give to the ESP32

    // BLEMidiClient.enableDebugging();  // Uncomment to see debugging messages from the library
    // delay(2000);  // useless delay. Safety measure to be able read all the debug stuff from Serial console.
}

void setup()
{
    pinMode(ONBOARD_LED, OUTPUT);
    
    Serial.begin(115200);
    TxRxSerial.begin(BAUD_RATE, SERIAL_8N1, 16, 17);
    UART_MIDI.begin(MIDI_CHANNEL_OMNI);

    init_ble();
    
}

void loop()
{

    if (BLEMidiClient.isConnected())
    {
        unblink();
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
           Serial.print("num bluetooth devices ");
        if (text_debug)
            Serial.println(nDevices);
        if (nDevices > 0)
        {
            // Searching for the one specific device we want to connect to ("Artiphon Orba 2")

            for (int i = 0; i < nDevices; i++)
            {
                if (text_debug)
                    Serial.println(BLEMidiClient.getScannedDevice(i)->getName().c_str());

                if (
                    (strcmp(BLEMidiClient.getScannedDevice(i)->getName().c_str(), DEVICE_TO_CONNECT1) == 0)  ||
                    (strcmp(BLEMidiClient.getScannedDevice(i)->getName().c_str(), DEVICE_TO_CONNECT2) == 0)  ||
                     connect_to_any)
                {
                    if (text_debug)
                     Serial.println("That's the one, let's try connecting");

                    if (BLEMidiClient.connect(0))
                    {
                        if (text_debug)
                            Serial.println("Connection established");
                    }
                    else
                    {
                        if (text_debug)
                            Serial.println("Connection failed");
                    }
                }
            }
            delay(3000); // We wait 3s before attempting a new connection
        }
    }
}
