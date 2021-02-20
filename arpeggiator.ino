
/*********************************************************************
 *                   Arpeggiating Glitch Synth                       *
 ********************************************************************* 
 * 
 * INPUTS:
 * Analog 1   - LDR
 * Analog 2   - Rotation Knob
 * Analog 3   - Rotation Knob
 * Analog 4   - Rotation Knob
 * Digital 4  - Pushbutton
 * Digital 5  - Pushbutton
 * Digital 8  - Pushbutton
 * Digital 12 - Pushbutton
 * 
 * OUTPUTS:
 * Digital 7  - LED
 * Digital 11 - LED
 * Digital 9  - Audio Output (Jumpers to Audio Jack)
 *
 *********************************************************************/

#include <MozziGuts.h>
#include <WavePacket.h>
#include <RollingAverage.h>
#include <AutoMap.h>

const int SENSOR_PIN   = 0; // Pin 0 used for floating value
const int KNOB_PIN     = 1; // Analog input pin 3
const int KNOB_PIN_1   = 3; // Analog input pin 2
const int KNOB_PIN_2   = 5; // Analog input pin 4
const int LDR1_PIN     = 11; // Analog input for fm_intensity to pin 1
const int BUTTON_PIN   = 12; // Digital input pin 12
const int BUTTON_PIN_1 = 4; // Digital input pin 4
const int BUTTON_PIN_2 = 5; // Digital input pin 5
const int BUTTON_PIN_3 = 8; // Digital input pin 8
const int LED_PIN      = 2; // Digital output pin 2 Red LED

int brightness = 0;    // How bright the LED is
int fadeAmount = 5; 

// A mess of variables with silly names but perfectly reasonable functions
int heck    = 3;
int dang    = 0;
int alright = 1;
int swotch  = 1;
int cons    = 4;
int button1 = 0;
int button2 = 0;
int button3 = 0;
int button4 = 0;
int con     = 4;
int fun     = 1;
int gosh    = 100;

// min and max values of synth parameters to map AutoRanged analog inputs to
const int MIN_F = 10;
const int MAX_F = 1000;
const int MIN_BW = 10;
const int MAX_BW = 1000;
const int MIN_CF = 60;
const int MAX_CF = 2000;
long randNumber;

// variables for reading the pushbutton status
int buttonState  = 0; 
int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;

// for smoothing the control signals
// use: RollingAverage <number_type, how_many_to_average> myThing
RollingAverage <int, 64> kAverageF;
RollingAverage <int, 64> kAverageBw;
RollingAverage <int, 64> kAverageCf;
AutoMap kMapF(0,1023,MIN_F,MAX_F);
AutoMap kMapBw(0,1023,MIN_BW,MAX_BW);
AutoMap kMapCf(0,1023,MIN_CF,MAX_CF);
WavePacket <DOUBLE> wavey; // DOUBLE selects 2 overlapping streams

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUTTON_PIN_1, INPUT);
  pinMode(BUTTON_PIN_2, INPUT);
  pinMode(BUTTON_PIN_3, INPUT);
  pinMode(LED_PIN, OUTPUT);    
  
  Serial.begin(115200);
  // wait before starting Mozzi to receive analog reads, so AutoRange will not get 0
  delay(100);
  startMozzi();
}

void updateControl() {
  int thirdknob = mozziAnalogRead(KNOB_PIN_2) + 1;
  int reading = mozziAnalogRead(SENSOR_PIN);
 
  float voltage = reading * 5.0;
  float temperatureC = (voltage - 0.5) * 100;
 
  int knobby = mozziAnalogRead(KNOB_PIN) + 1;
  int knibby = mozziAnalogRead(KNOB_PIN_1) + 1;
  int knab = knibby / 10;
 
  buttonState = digitalRead(BUTTON_PIN);
  buttonState1 = digitalRead(BUTTON_PIN_1);
  buttonState2 = digitalRead(BUTTON_PIN_2);
  buttonState3 = digitalRead(BUTTON_PIN_3);
 
  int bandwidth = mozziAnalogRead(LDR1_PIN);
  int fundamental = bandwidth ;
 
  fundamental = kMapF(fundamental);
  bandwidth = kMapBw(bandwidth);
 
  int centre_freq = 250;
 
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);
 
  if (alright < 8) {
    brightness = 0;    // how bright the LED is
    fadeAmount = 5; 
    centre_freq = 0;
    bandwidth = 0;
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN, HIGH);
  } 
  
  if (alright<con) {
    brightness = brightness + fadeAmount;
    // reverse the direction of the fading at the ends of the fade:
    if (brightness == 0 || brightness == 100) {
      fadeAmount = -fadeAmount * 4;
    }
  }
  
  if (buttonState1 == HIGH) {
    button1 = button1 + 1;
    if (button1 > 250) {
      button1 = 1;
    }
  }

  if (buttonState2 == HIGH) {
    button2 = button2 + 1;
    if (button2 > 250) {
      button2 = 1;
    }
  }
  
  if (buttonState == HIGH) {
    button3 = button3 + 1;
    if (button3 > 250) {
      button3 = 1;
    }
  }

  alright = alright + 1;
  con = (thirdknob / 20) + 4;

  if (alright > 7) {
    alright = 0;
    swotch = swotch + 10;
  }
  
  fundamental = fundamental / 4;
  brightness = brightness + con;
  centre_freq = centre_freq - con;

  if (swotch > 9 && swotch < 20) {
    fundamental = fundamental + button1;
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);
  }
  
  if (swotch > 19 && swotch < 30) {
    fundamental = fundamental + button2;
    brightness = brightness - con;
    centre_freq = centre_freq + con;
  }
  
  if (swotch > 29 && swotch < 40) {
    fundamental = fundamental + button3;
    brightness = brightness + con;
    centre_freq = centre_freq - con;
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);   
  }
  
  if (swotch > 39) {
    swotch = 0;
    brightness = brightness - con;
    centre_freq = centre_freq + con;
  }
  
  if (buttonState3 == HIGH) {
    randNumber = random(300);
    button1 = randNumber;
    randNumber = random(300);
    button2 = randNumber;
    randNumber = random(300);
    button3 = randNumber;
  } 
  
  cons = (con / 10) + 4;
  bandwidth = (brightness * 14) + (knobby / 2);
  centre_freq = (brightness + knibby * 3);
  wavey.set(fundamental, bandwidth, centre_freq);  
}

int updateAudio() {
  while (alright < cons) {
    return wavey.next() >> 8; // >> 8 for AUDIO_MODE STANDARD
  }
}

void loop() {
  audioHook(); // required here  
}
