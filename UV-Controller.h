//DEFINE INPUTS
#define START_BUTTON 2
#define MODE_SW_1 4
#define MODE_SW_2 7
#define PIR_SENSOR 10
#define SHIELD_SW 11
#define DEADMAN_SW 12


//DEFINE OUTPUTS
#define LAMPS 8
#define BUZZER 9

//MODES
#define MAN 3
#define T10 1
#define T20 2

//Sensor Structure
struct SENSORS_STRUCT {
  int pir;
  int deadman;
  int mode;
  int shield;
};
//DEBOUNCERS
Bounce modesw1_debouncer = Bounce();
Bounce modesw2_debouncer = Bounce();
Bounce pir_debouncer = Bounce();
Bounce shield_debouncer = Bounce();
Bounce deadman_debouncer = Bounce();


//Timer para la lampara
Countimer UVtimer;

/*** AUTO MODE ***/
bool desinfeccionEnProceso = false;
bool modoDesinfeccion = false;


//Prototypes
void readSensors(void);
void manualMode(void);
void autoMode(void);
void printSensors(void);
void onComplete(void);


/***********************************************************************
   AUXILIARY CLASSES
*/
class Flasher
{
    // Class Member Variables
    // These are initialized at startup
    int ledPin;      // the number of the LED pin
    long OnTime;     // milliseconds of on-time
    long OffTime;    // milliseconds of off-time

    // These maintain the current state
    int ledState;                 // ledState used to set the LED
    unsigned long previousMillis;   // will store last time LED was updated

    // Constructor - creates a Flasher
    // and initializes the member variables and state
  public:
    Flasher(int pin, long on, long off)
    {
      ledPin = pin;
      pinMode(ledPin, OUTPUT);

      OnTime = on;
      OffTime = off;

      ledState = LOW;
      previousMillis = 0;
    }

    void Pattern(long on, long off) {
      OnTime = on;
      OffTime = off;
    }

    void Update()
    {
      // check to see if it's time to change the state of the LED
      unsigned long currentMillis = millis();
      if (!(OnTime == 0)) {

        if ((ledState == HIGH) && (currentMillis - previousMillis >= OnTime) && (OffTime != 0))
        {
          ledState = LOW;  // Turn it off
          previousMillis = currentMillis;  // Remember the time
          digitalWrite(ledPin, ledState);  // Update the actual LED
        }
        else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime) && (OnTime != 0))
        {
          ledState = HIGH;  // turn it on
          previousMillis = currentMillis;   // Remember the time
          digitalWrite(ledPin, ledState);   // Update the actual LED
        }
      }
      else {
        digitalWrite(ledPin, LOW);
      }
    }
};
