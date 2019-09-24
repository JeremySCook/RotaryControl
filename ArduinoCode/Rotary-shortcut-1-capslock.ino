//Need to include i2cEncoderLibV2.cpp and i2cEncoderLibV2.h in same directory

//Initial setup =============================================================================

#include <Wire.h>
#include "i2cEncoderLibV2.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include "HID-Project.h"

#define PINRGB  4
#define NUMPIXELSRGB  12

int xMovements[] = {-3, -8, -14, -16, -19, -19, -16, -14, -8, -3, 3, 8, 14, 16, 19, 19, 16, 14, 8, 3, -3};
int yMovements[] = {19, 16, 14, 8, 3, -3, -8, -14, -16, -19, -19, -16, -14, -8, -3, 3, 8, 14, 16, 19, 19}; 
int controlMode = 0;
int sensorValue;
int sensorValue1;

i2cEncoderLibV2 Encoder(0x00);
const int IntPin = A2; // Definition of the interrupt pin on PICO

Adafruit_NeoPixel pixelsRGB = Adafruit_NeoPixel(NUMPIXELSRGB, PINRGB, NEO_RGB + NEO_KHZ800);

//End Initial Setup ==========================================================================

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(A0, INPUT_PULLUP); //define analog pin 0 as pullup input
  pinMode(A1, INPUT_PULLUP); //define analog pin 1 as pullup input
  pinMode(IntPin, INPUT); //sets A2 as int pin for I2C encoder
  pixelsRGB.begin(); // This initializes the NeoPixel library as pixelsRGB object.

//setup for encoder
  Encoder.begin(INT_DATA | WRAP_ENABLE | DIRE_LEFT | IPUP_ENABLE | RMOD_X1 | STD_ENCODER);
  Encoder.writeCounter((int32_t)0); /* Reset the counter value */
  Encoder.writeMax((int32_t)19); /* Set the maximum threshold*/
  Encoder.writeMin((int32_t) 0); /* Set the minimum threshold */
  Encoder.writeStep((int32_t)1); /* Set the step to 1*/
  Encoder.writeInterruptConfig(0xff); /* Enable all the interrupt */
  Encoder.writeAntibouncingPeriod(20);  /* Set an anti-bouncing of 200ms */
  Encoder.writeDoublePushPeriod(50);  /*Set a period for the double push of 500ms */

//setup keyboard and mouse input
Mouse.begin();
Keyboard.begin();
}

//Start Main Loop =========================================================================

void loop() {

//caps lock code ==========================================================================

if (BootKeyboard.getLeds() & LED_CAPS_LOCK){
  Serial.println("capsLock");
  for(int x = 0; x <12; x++){
  pixelsRGB.setPixelColor(x, pixelsRGB.Color(0,0,10)); 
  }
  pixelsRGB.show(); // This sends the updated pixel color to the hardware.
}

//lights loop in both modes ===============================================================

else {
  int pixelNumber = map(Encoder.readCounterByte(), 0, 19, 0, 11);
  for(int x = 0; x <12; x++){
  pixelsRGB.setPixelColor(x, pixelsRGB.Color(0,0,0)); // Red *red/green may be reversed depending on your strip
  }
  pixelsRGB.setPixelColor(pixelNumber, pixelsRGB.Color(10,0,0));
  if (pixelNumber != 11) pixelsRGB.setPixelColor(pixelNumber + 1, pixelsRGB.Color(0,10,0));
  if (pixelNumber == 11) pixelsRGB.setPixelColor(0, pixelsRGB.Color(0,10,0));
  if (pixelNumber != 0) pixelsRGB.setPixelColor(pixelNumber - 1, pixelsRGB.Color(0, 10, 0));
  if (pixelNumber == 0) pixelsRGB.setPixelColor(11, pixelsRGB.Color(0, 10, 0));  
  pixelsRGB.show(); // This sends the updated pixel color to the hardware.
}

//end NeoPixel lighting code===================================================================

  sensorValue = digitalRead(A0);
  sensorValue1 = digitalRead(A1);
  uint8_t enc_cnt; //read encoder input
  //Serial.print("sensorValue ");
  //Serial.println(sensorValue);
  //Serial.print("sensorValue1 ");
  //Serial.println(sensorValue1);
  //Serial.print("EncoderPUSHP ");
  //Serial.println(Encoder.readStatus(PUSHP));
  //Serial.print("controlMode ");
  //Serial.println(controlMode);
  delay(10);
  if (controlMode == 0) volume();
  if (controlMode == 1) game();
}

void volume() {
  if (sensorValue == 0 && sensorValue1 == 0){ 
      controlMode = 1;
      delay(250);
      }
  else if (digitalRead(IntPin) == LOW) {
    if (Encoder.updateStatus()) {
      if (Encoder.readStatus(RINC)) {
        Serial.print("Increment: ");
        Serial.println(Encoder.readCounterByte());
        Consumer.write(MEDIA_VOLUME_UP);
        delay(1);
      }
    if (Encoder.readStatus(RDEC)) {
        Serial.print("Decrement: ");
        Serial.println(Encoder.readCounterByte());
        Consumer.write(MEDIA_VOLUME_DOWN);
        delay(1);
      }
    if (Encoder.readStatus(PUSHR)) {
        Serial.println("Push button Released");
        delay(1);        
      }

    if (Encoder.readStatus(PUSHP)) {
        Serial.println("Push button Pressed");
        Consumer.write(MEDIA_PLAY_PAUSE);
        delay(1); 
      }
    }
  }
  else if (sensorValue1 == 0){
      Consumer.write(MEDIA_NEXT);
      delay(250);
      
  }
  else if (sensorValue == 0){
      Consumer.write(MEDIA_PREVIOUS);
      delay(250);
  }    
}

void game() {
//homing routine =====================================================  
  if (sensorValue == 0 && sensorValue1 == 0){
    Mouse.move(60, 0);
    delay(250);
  }
//end homing routine =================================================
  sensorValue = digitalRead(A0);
  sensorValue1 = digitalRead(A1);
  uint8_t enc_cnt; //read encoder input
  if (digitalRead(IntPin) == LOW) {
    if (Encoder.updateStatus()) {
      if (Encoder.readStatus(RINC)) {
        Serial.print("Increment: ");
        Serial.println(Encoder.readCounterByte());
        Mouse.move(xMovements[Encoder.readCounterByte()], yMovements[Encoder.readCounterByte()]);
        Serial.println(xMovements[Encoder.readCounterByte()]);
        Serial.println(yMovements[Encoder.readCounterByte()]);
        delay(1);
      }
    if (Encoder.readStatus(RDEC)) {
        Serial.print("Decrement: ");
        Serial.println(Encoder.readCounterByte());
        Mouse.move(-xMovements[Encoder.readCounterByte()+1], -yMovements[Encoder.readCounterByte()+1]);
        Serial.println(Encoder.readCounterByte());
        Serial.println(-xMovements[Encoder.readCounterByte()+1]);
        Serial.println(-yMovements[Encoder.readCounterByte()+1]);
        delay(1);
      }
    if (Encoder.readStatus(PUSHP)) {
        Serial.println("Push button Pressed");
        if (sensorValue == 0 && sensorValue1 == 0 ){ 
        controlMode = 0;
        delay(250);
      }
        else {
        Mouse.press();
        delay(1);
        } 
      }
    if (Encoder.readStatus(PUSHR)) {
        Serial.println("Push button Released");
        Mouse.release();
        delay(1);        
      }
    }  
}
}
