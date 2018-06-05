//Libraries
#include <Adafruit_NeoPixel.h>  //Library to simplify interacting with the LED strand
#include <math.h> // used for sine function

//Constants
#define LED_PIN   A5  //Pin for the pixel strand. Does not have to be analog.
#define KNOB_PIN  A1  //Pin for the trimpot 10K
#define LED_TOTAL 60  //Change this to the number of LEDs in your strand.

//Declare Spectrum Shield pin connections
#define STROBE 4
#define RESET 5
#define DC_One A0
#define DC_Two A1 

//Globals
Adafruit_NeoPixel strand = Adafruit_NeoPixel(LED_TOTAL, LED_PIN, NEO_GRB + NEO_KHZ800);  //LED strand object
int position = 0; // position on the strand, used to cycle light around the strand

long previousMillis = 0;        // will store last time run() was called
long interval = 100;           // interval at which to blink (milliseconds)

float knob = 1023.0;   //Holds the percentage of how twisted the trimpot is. Used for adjusting the max brightness.

int waves = 2;

bool goTime = false;

bool buttonStates[3];

bool breatheOn = false;

//Define spectrum variables
int freq_amp;
int Frequencies_One[7];
int Frequencies_Two[7]; 
int i;

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

  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);

  //Set spectrum Shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);  
  digitalWrite(STROBE, HIGH);
  digitalWrite(RESET, HIGH);
  
  //Initialize Spectrum Analyzers
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, HIGH);
  delay(1);
  digitalWrite(STROBE, HIGH);
  delay(1);
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, LOW);

buttonStates[0] = false;
buttonStates[1] = false;  
buttonStates[2] = false;

}

SIGNAL(TIMER0_COMPA_vect) {
  goTime = true;
}


void loop() {

    knob = analogRead(KNOB_PIN) / 1023.0; //Record how far the trimpot is twisted
    waves = knobToFactors120(); 


    checkButton();
    
    if (goTime) {
      goTime= false;
      
      unsigned long currentMillis = millis(); 
      if(currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;   
        run();     
        
        Read_Frequencies();
  //      Graph_Frequencies();
        
      }
    
    }
  
}

void checkButton() {
    for (int i = 0; i < 3; i++) {
      if ((digitalRead(8+i) == LOW) && !buttonStates[i] ) // Button #1 pressed
      {
        Serial.println("Button Pressed");
        buttonStates[i] = true;
      }
      if ((digitalRead(8+i) == HIGH) && buttonStates[i] ) // Button #1 pressed
      {
        buttonStates[i] = false;
        Serial.println("Button Released");
        if (i == 0){
           toggleBreathe();
        }
        else if (i == 1) {
          Serial.println("Two");
        }
        else if (i == 2){
          Serial.println("Three");
        }
      }
    }
}

void toggleBreathe() {
  if (breatheOn) {
    breatheOn = false;
    Serial.println("Breathe Off");
  }
  else { 
    breatheOn = true;
    Serial.println("Breathe On");
  }
}

void run() {
  float breathe;
  if (breatheOn) {
     breathe = ((sin(position*2*3*PI/180)+1)/2*0.4)+0.2;  
  } else {
    breathe = 1;
  }

  
  int freak[7];
  
  for( i= 0; i<7; i++) {
     if(Frequencies_Two[i] > Frequencies_One[i]){
        freak[i] = Frequencies_Two[i];
     }
     else{
        freak[i] = Frequencies_One[i];
     }
   }

//  freak [6] = 1023;
    
  for (int i = 0; i < LED_TOTAL; i++) {
    
    int r = (int)((sin(i*waves*3*PI/180)+1)*255/2);
    int g = (int)((sin(i*waves*3*PI/180+2*PI/3)+1)*255/2);
    int b = (int)((sin(i*waves*3*PI/180+4*PI/3)+1)*255/2);
    
//    int r = (int)((sin(i*waves*3*PI/180)+1)*255/2 * freak[0]/1023);
//    int r = 0;
//    int g = (int)((sin(i*waves*3*PI/180+2*PI/3)+1)*255/2 * freak[2]/1023);
//    int g = 0;
//    int b = (int)((sin(i*waves*3*PI/180+4*PI/3)+1)*255/2 * freak[1]/1023);
//    int b = 0;
    
    strand.setPixelColor(((i) % LED_TOTAL), strand.Color(r*breathe,g*breathe,b*breathe));
//    strand.setPixelColor(((i + position) % LED_TOTAL), strand.Color(r*breathe,g*breathe,b*breathe));
  }
   
  position++;
  if (position > LED_TOTAL-1) position %= LED_TOTAL;
  
  //This command actually shows the lights. If you make a new visualization, don't forget this!
  strand.show();
}

/*******************Pull frquencies from Spectrum Shield********************/
void Read_Frequencies(){
  //Read frequencies for each band
  for (freq_amp = 0; freq_amp<7; freq_amp++)
  {
    Frequencies_One[freq_amp] = analogRead(DC_One);
    Frequencies_Two[freq_amp] = analogRead(DC_Two); 
    digitalWrite(STROBE, HIGH);
    digitalWrite(STROBE, LOW);
  }
}

/*******************Light LEDs based on frequencies*****************************/
void Graph_Frequencies(){
   for( i= 0; i<7; i++)
   {
     if(Frequencies_Two[i] > Frequencies_One[i]){
        Serial.println(Frequencies_Two[i]);
     }
     else{
        Serial.println(Frequencies_One[i]);
     }
   }
}

// this function takes the knob value and returns a factor of 120
// which ensures there is always an equal number of off LEDs between on LEDs
uint8_t knobToFactors120() {
  if (knob > (float)15/(float)16) return 120;
  if (knob > (float)14/(float)16) return 60;
  if (knob > (float)13/(float)16) return 40;
  if (knob > (float)12/(float)16) return 30;
  if (knob > (float)11/(float)16) return 24;
  if (knob > (float)10/(float)16) return 20;
  if (knob > (float)9/(float)16) return 15;
  if (knob > (float)8/(float)16) return 12;
  if (knob > (float)7/(float)16) return 10;
  if (knob > (float)6/(float)16) return 8;
  if (knob > (float)5/(float)16) return 6;
  if (knob > (float)4/(float)16) return 5;
  if (knob > (float)3/(float)16) return 4;
  if (knob > (float)2/(float)16) return 3;
  if (knob > (float)1/(float)16) return 2;
  return 1;
}
