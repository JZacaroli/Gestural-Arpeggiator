/* Get the accelerometer values and send them over to processing.
     Processing then buffers the values and sends them over to the WEKINATOR.
     The WEKINATOR then processes the buffer and sends output to MAX/MSP.
     Over-architected? Nah. */

#include <Wire.h>                 // Must include Wire library for I2C
#include "SparkFun_MMA8452Q.h"    // Click here to get the library: http://librarymanager/All#SparkFun_MMA8452Q

MMA8452Q accel;                   // create instance of the MMA8452 class

// We only start transmitting via bluetooth when told via MaxMSP.
int started = 0;
int serialValue;

//To distinguish whether the Instrument is on.
int onLed = 12;
int ledOn = 1;
int t = 0;
// How fast the led is blinking tells us information
// Really fast = turning on. Slowly = on but not sending BT data. Medium = Sending BT data.
int lightBlinkSlowness = 15;

//For use in filtering the first acceletometer's values
float filteredX1 = 0; float filteredY1 = 0; float filteredZ1 = 0;
float prevX1 = 0, prevY1 = 0, prevZ1 = 0;
float prevprevX1 = 0, prevprevY1 = 0, prevprevZ1 = 0;

//For use in filtering the second accelerometer's values
float filteredX2 = 0, filteredY2 = 0, filteredZ2 = 0;
float prevX2 = 0, prevY2 = 0, prevZ2 = 0;
float prevprevX2 = 0, prevprevY2 = 0, prevprevZ2 = 0;

//Message to send to processing is of the form:
//x1,x2,y1,y2,z1,z2,X1,X2,Y1,Y2,Z1,Z2
boolean sendSerial = true; //only send serial message every other time.

//Stuff for microphone
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

void setup() {
  pinMode(onLed, OUTPUT);
  Serial.begin(9600);
  Wire.begin();
  accel.begin();

  turningOnLightSequence();
}


void loop() {

  if (Serial.available()) // check to see if there's serial data in the buffer
  {
    while (Serial.available()) {
      serialValue = Serial.read(); // read a byte of serial data until the buffer is empty.
    }
    started = 1 - started; // An on/off switch, effectively.
    lightBlinkSlowness = 20 - lightBlinkSlowness;
  }

  blinkOnLight();

  delay(50); //BT doesn't work sending quicker than every 50ms

  if (started) {
    /* Read the microphone signal */
    //double micValue = readMicrophoneSignal();

    /* First Acceletometer (I2C) */
    float unfilteredX1 = 0, unfilteredY1 = 0, unfilteredZ1 = 0;
    if (accel.available()) {
      // Acceleration of x, y, and z directions in g units. Scaled to between 0 and 1.
      unfilteredX1 = 0.5 + (float(accel.getCalculatedX()) / 4);
      unfilteredY1 = 0.5 + (float(accel.getCalculatedY()) / 4);
      unfilteredZ1 = 0.5 + (float(accel.getCalculatedZ()) / 4);
      /*filteredX1 = 0.05 * prevprevX1 + 0.15 * prevX1 + 0.25 * filteredX1 + 0.55 * unfilteredX1;
        filteredY1 = 0.05 * prevprevY1 + 0.15 * prevY1 + 0.25 * filteredY1 + 0.55 * unfilteredY1;
        filteredZ1 = 0.05 * prevprevZ1 + 0.15 * prevZ1 + 0.25 * filteredZ1 + 0.55 * unfilteredZ1;*/
    }

    /* Second Accelerometer (Analog Read) */
    float unfilteredX2 = constrain((1.5 * float(analogRead(A0)) / 675) - 0.25, 0, 1); //Scale the inputs to between 0-1. Better for ML.
    float unfilteredY2 = constrain((1.5 * float(analogRead(A1)) / 675) - 0.25, 0, 1); //Also constraining it to be between -2g and 2g because that's what the other accelerometer uses.
    float unfilteredZ2 = constrain((1.5 * float(analogRead(A2)) / 675) - 0.25, 0, 1);
    /*filteredX2 = 0.05 * prevprevX2 + 0.15 * prevX2 + 0.25 * filteredX2 + 0.55 * unfilteredX2;
      filteredY2 = 0.05 * prevprevY2 + 0.15 * prevY2 + 0.25 * filteredY2 + 0.55 * unfilteredY2;
      filteredZ2 = 0.05 * prevprevZ2 + 0.15 * prevZ2 + 0.25 * filteredZ2 + 0.55 * unfilteredZ2;*/

    /**
       Message to send to processing is of the form:
       x1,x2,y1,y2,z1,z2,X1,X2,Y1,Y2,Z1,Z2
       Capitals are used to denote the MMA8452Q accelerometer vals.
    */
    if (sendSerial) {
      /*Serial.println(String(prevX2) + "," + String(filteredX2) + "," + String(prevY2) + "," + String(filteredY2) + "," + String(prevZ2) + "," + String(filteredZ2)
                     + "," + String(prevX1) + "," + String(filteredX1) + "," + String(prevY1) + "," + String(filteredY1) + "," + String(prevZ1) + "," + String(filteredZ1));
        //+ "," + String(micValue));*/
      Serial.println(String(unfilteredX2) + "," + String(unfilteredY2) + "," + String(unfilteredZ2) + ","
                     + String(unfilteredX1) + "," + String(unfilteredY1) + "," + String(unfilteredZ1));// + "," + String(micValue));
    }
    //sendSerial = !sendSerial;

    /**
       Update the 1st filter's values
    */
    /*prevprevX1 = prevX1;
      prevprevY1 = prevY1;
      prevprevZ1 = prevZ1;
      prevX1 = unfilteredX1;
      prevY1 = unfilteredY1;
      prevZ1 = unfilteredZ1;*/
    /**
       Update the 2nd filter's values
    */
    /*prevprevX2 = prevX2;
      prevprevY2 = prevY2;
      prevprevZ2 = prevZ2;
      prevX2 = unfilteredX2;
      prevY2 = unfilteredY2;
      prevZ2 = unfilteredZ2;*/
  }
}

/**
  Blink the light fast so we know when it's turning on.
*/
void turningOnLightSequence() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(onLed, HIGH);
    delay(50);
    digitalWrite(onLed, LOW);
    delay(50);
  }
}

/**
   Blink the light at a
*/
void blinkOnLight() {
  t++;
  if (t % lightBlinkSlowness == 0) {
    t -= lightBlinkSlowness;
    ledOn = 1 - ledOn;
    if (ledOn == 1) {
      digitalWrite(onLed, HIGH);
    } else {
      digitalWrite(onLed, LOW);
    }
  }
}

double readMicrophoneSignal() {
  unsigned long startMillis = millis(); // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for sampleWindow number of mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(A5);
    if (sample < 1024) { // toss out spurious readings
      if (sample > signalMax) {
        signalMax = sample;  // save just the max levels
      } else if (sample < signalMin) {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  double micValue = (peakToPeak * 1.0) / 1024;  // convert to between 0 and 1.
  return micValue;
}
