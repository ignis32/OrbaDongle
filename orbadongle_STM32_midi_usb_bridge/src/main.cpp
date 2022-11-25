#include <Arduino.h>

#include <USBComposite.h>
USBMIDI midi;
USBCompositeSerial CompositeSerial;

#define ONBOARD_LED PB2

void init_usb_stuff()
{
  midi.registerComponent();
  CompositeSerial.registerComponent();
  USBComposite.begin();
  //while (!USBComposite); // might hang on this moment if nobody connects. Should be disabled for prod build probably
  USBComposite.setProductId(0x0031);

  Serial2.begin(31250);
}


void setup()
{
    init_usb_stuff();
     pinMode(PB2, OUTPUT);
    digitalWrite(PB2, HIGH);
}

int blink_counter = 0;
void blink() // temporary disable led on each event, to indicate activity.
{
    blink_counter = 5000;  // number of loops before enabling led back.
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

void loop()
{

 unblink();

 while (Serial2.available() > 0)
 {
  blink();
  //CompositeSerial.write(".");
  CompositeSerial.write(Serial2.read());
  
 }
  
   
  
}