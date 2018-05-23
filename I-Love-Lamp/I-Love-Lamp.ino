//Libraries
#include <Adafruit_NeoPixel.h>  //Library to simplify interacting with the LED strand
#include <math.h> // used for sine function

//Constants
#define LED_PIN   A5  //Pin for the pixel strand. Does not have to be analog.
#define KNOB_PIN  A1  //Pin for the trimpot 10K
#define LED_TOTAL 120  //Change this to the number of LEDs in your strand.

//Globals
Adafruit_NeoPixel strand = Adafruit_NeoPixel(LED_TOTAL, LED_PIN, NEO_GRB + NEO_KHZ800);  //LED strand object
int position = 0; // position on the strand, used to cycle light around the strand

long previousMillis = 0;        // will store last time run() was called
long interval = 100;           // interval at which to blink (milliseconds)

float knob = 1023.0;   //Holds the percentage of how twisted the trimpot is. Used for adjusting the max brightness.

bool goTime = false;

//Standard Functions
void setup() {    //Like it's named, this gets ran before any other function.

  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);

  Serial.begin(9600); //Sets data rate for serial data transmission.
  Serial.println("Setup RainbowSine");
  strand.begin(); //Initialize the LED strand object.
  strand.show();  //Show a blank strand, just to get the LED's ready for use. 
}

SIGNAL(TIMER0_COMPA_vect) {
  goTime = true;
}


void loop() {
  
  if (goTime) {
    goTime= false;
    
    unsigned long currentMillis = millis(); 
    if(currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;   
      run();     
    }
  
  }
}


void run() {
//  float breathe = ((sin(position*2*3*PI/180)+1)/2*0.8)+0.2;
  float breathe = 1;
  
  for (int i = 0; i < LED_TOTAL; i++) {
    
    int waves = 4;
    int r = (int)((sin(i*waves*3*PI/180)+1)*255/2);
    int g = (int)((sin(i*waves*3*PI/180+2*PI/3)+1)*255/2);
    int b = (int)((sin(i*waves*3*PI/180+4*PI/3)+1)*255/2);
    
//    strand.setPixelColor(((i) % LED_TOTAL), strand.Color(r*breathe,g*breathe,b*breathe));
    strand.setPixelColor(((i + position) % LED_TOTAL), strand.Color(r*breathe,g*breathe,b*breathe));
  }
   
  position++;
  if (position > LED_TOTAL-1) position %= LED_TOTAL;
  
  //This command actually shows the lights. If you make a new visualization, don't forget this!
  strand.show();
}

