import processing.serial.*;
import oscP5.*;
import netP5.*;

//Used in Serial Communication with Arduino
Serial myPort;
String serial;
int endOfSerialMessage = 10;

//Used in OSC Communication with Wekinator
OscP5 oscP5;
NetAddress dest;

//Buffer of data to send to Wekinator
ArrayList<Float> wekiBuffer = new ArrayList<Float>();
int numberOfTimeStepsPerWekiMessage = 1;
int numberOfValuesPerSerialMessage = 6;
int wekiBufferSize = numberOfValuesPerSerialMessage*numberOfTimeStepsPerWekiMessage;

//Misc variables
//int t = 0;
boolean started = false;

void setup() {
  size(800, 500);

  //Serial stuff to receive from Arduino
  printArray(Serial.list());
  String portName = Serial.list()[1];
  myPort = new Serial(this, portName, 9600);

  //Osc Stuff to send to the Wekinator
  oscP5 = new OscP5(this, 9000);
  dest = new NetAddress("127.0.0.1", 6448);
}

void draw() {
  background(0);
  //Draw a green/red box on screen depending on if we've started or not.
  if (!started) {
    fill(255, 0, 0);
  } else {
    fill(0, 255, 0);
  }
  rect(100, 100, 600, 300);
  drawHeights();
  drawStartStopText();

  // If the buffer has gotten too big, remove the oldest elements (FIFO)
  while (wekiBuffer.size() > wekiBufferSize-numberOfValuesPerSerialMessage)
  {
    wekiBuffer.remove(0);
  }

  //If the port is available then read 
  if (myPort.available() > 0) {
    serial = myPort.readStringUntil(endOfSerialMessage);
    if (serial != null) {
      //println("serial: " + serial);
      String[] arduinoData = split(serial, ',');
      for (int i=0; i<arduinoData.length; i++) {
        wekiBuffer.add(float(arduinoData[i]));
      }
    }
  }
  //If we've successfully read enough values then we should send them over!
  if (wekiBuffer.size() == wekiBufferSize) {
    sendOsc();
    //println("Sending OSC Message now: " + t);
    //t++;
    //printArray(wekiBuffer);
  }
}

void sendOsc() {
  OscMessage msg = new OscMessage("/wek/inputs");
  for (int i=0; i<wekiBuffer.size(); i++) {
    msg.add(wekiBuffer.get(i));
  }
  oscP5.send(msg, dest);
}

/**
 * Draw the accelerometer values in a bar chart type fashion.
 */
void drawHeights() {
  fill(0, 0, 255);
  for (int i=0; i<wekiBuffer.size(); i++) {
    float h = wekiBuffer.get(i)*300;
    rect(100 * (i+1), 400 - h, 100, h);
  }
}

/**
 * Tell the user what to press to start sending messages to the Wekinator.
 */
void drawStartStopText() {
  textSize(32);
  fill(0, 102, 153);
  if (started) {
    text("Press ENTER to stop!", 10, 60);
  } else {
    text("Press ENTER to start!", 10, 60);
  }
}

/**
 * The Enter key starts sending messages to the Wekinator.
 */
void keyPressed() {
  if (key == ENTER) {
    started = !started;
    myPort.write(65);
  }
}
