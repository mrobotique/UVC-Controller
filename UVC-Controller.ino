/*
  Requiere
  Push button library:
  https://github.com/marcobrianza/ClickButton
  Debouncer:
  https://github.com/thomasfredericks/Bounce2
  Contdown timer
  https://github.com/inflop/Countimer
*/

#include <Bounce2.h>
#include <Countimer.h>
#include "UV-Controller.h"
#include "ClickButton.h"

//The start button is kind of special so,
//It is intialized in a sightly different way
ClickButton User_button(START_BUTTON, LOW, CLICKBTN_PULLUP);
int button_state_machine = 0;

//Declaracion de sensores
// La estructura contiene pir, deadman, mode, shield
SENSORS_STRUCT sensors = {0, 0, 0, 0};
Flasher buzzer(BUZZER, 0, 0);

void setup() {
  // Serial Para debuguear
  Serial.begin(115200);
  //Inicializa las entradas
  pinMode(MODE_SW_1, INPUT_PULLUP);
  pinMode(MODE_SW_2, INPUT_PULLUP);
  pinMode(PIR_SENSOR, INPUT_PULLUP);
  pinMode(SHIELD_SW, INPUT_PULLUP);
  pinMode(DEADMAN_SW, INPUT_PULLUP);
  //Inicializa las salidas
  pinMode(LAMPS, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Declarar las instancias para el debouncer
  modesw1_debouncer.attach(MODE_SW_1);
  modesw1_debouncer.interval(300); // intervalo en ms
  modesw2_debouncer.attach(MODE_SW_2);
  modesw2_debouncer.interval(300); // intervalo en ms
  pir_debouncer.attach(PIR_SENSOR);
  pir_debouncer.interval(20); // intervao en ms
  shield_debouncer.attach(SHIELD_SW);
  shield_debouncer.interval(500); // intervalo en ms
  deadman_debouncer.attach(DEADMAN_SW);
  deadman_debouncer.interval(1000); // intervalo en ms
}

/*** MAIN LOOP ***/
void loop() {
  readSensors();
  if (sensors.mode == MAN) manualMode();
  autoMode();
}

/*********************   Funciones ********************************/
/*** MANUAL MODE***/
void manualMode(void) {
  bool beep_on = false;

  delay(300); //Solo para evitar que se prendan las lamparas cuando el sw de modo de operacion pase de 10 a 20min
  while (sensors.mode == MAN) {
    readSensors(); //Actualiza el estado de los sensores
    buzzer.Update(); //Tick para el estado del buzzer
    // Serial.println("MAN"); //Debug
    if (sensors.shield == 1) { //Verifica que el escudo este colocado
      if (sensors.deadman == 1) { // Verifica que el switch de deadman este activado

        buzzer.Pattern(300, 1200);// Establece el patron de beep de la alarma audible
        // save the last time you blinked the LED
        digitalWrite(LAMPS, HIGH); // Enciende las lamparas
      }
      else {
        //Si el deadman switch ha estado desactivado por mas del intervalo permitido
        //apaga las UV y la alarma
        digitalWrite(LAMPS, LOW);
        buzzer.Pattern(0, 0);
      }
    } // if sensor shield
    else { //Si el escudo protector no esta colocado
      digitalWrite(LAMPS, LOW); //Apaga las luces

      if (sensors.deadman == 1) { //Si no hay escudo y el usuario oprime el deadman switch
        buzzer.Pattern(100, 10); // haz mucho ruido
      }
      else {
        buzzer.Pattern(0, 0);
      }
    }
  }
  digitalWrite(LAMPS, LOW); //Al salir del modo manual se apagan las luces

  buzzer.Pattern(0, 0); // y se apaga el buzzer
  buzzer.Update();
}

/*** AUTO MODE ***/
void autoMode() {
  //Timers
  unsigned long previousMillis = 0;
  unsigned long currentMillis = 0;
  long interval = 0;
  int tiempoExposicion = 0;
  bool pirWasTriggered = false;

  while (sensors.mode != MAN) { //Mientras este en automatico (automatico == no manual)
    readSensors(); //Actualiza el estado de los sensores
    buzzer.Update(); //Tick para el estado del buzzer
    User_button.Update(); //Tick para el estado del boton
    //Recupera el numero de clicks del  boton principal
    if (User_button.clicks != 0) button_state_machine = User_button.clicks;
    if (User_button.clicks >= 1) { // Si el usuario aprieta el boton una o multiples veces, todo se apaga
      digitalWrite(LAMPS, LOW);
      modoDesinfeccion = false;
      desinfeccionEnProceso = false;
      buzzer.Pattern(0, 0); // y se apaga el buzzer
    }
    // Si el usuario ejecuta un push largo
    if (button_state_machine == -1 && User_button.depressed == true)
    {
      //dos beeps para confirmar que el boton ha sido apretado por mucho tiempo
      for (int i = 0; i < 2; i++) {
        // Esta funcion es bloqueante. Aqui no importa mucho no hacerlo realtime
        // ya que el usuario ni siquiera notara la espera de 140ms
        digitalWrite(BUZZER, HIGH);//enciende el buzzer
        delay(35); //espera 35ms
        digitalWrite(BUZZER, LOW); //apaga el buzzer
        delay(35);//espera 35ms
      }
      while (User_button.depressed) { //No hacer nada mientras el usuario no suelta el boton
        User_button.Update();
      }
      //Si las lamparas estaban prendidas, entonce se apagan
      if (digitalRead(LAMPS)) {
        digitalWrite(LAMPS, LOW);
        modoDesinfeccion = false;
        desinfeccionEnProceso = false;
        buzzer.Pattern(0, 0); // y se apaga el buzzer
      }
      else {
        //Cargar el tiempo de exposicion
        if (sensors.mode == 1) {
          tiempoExposicion = 10; //10 minutos
          digitalWrite(BUZZER, HIGH);//enciende el buzzer
          delay(1000); //espera 35ms
          digitalWrite(BUZZER, LOW); //apaga el buzzer
          delay(800);//Evita el empalme de los bips
        }
        else if  (sensors.mode == 2) {
          tiempoExposicion = 20; //20 minutos
          for (int i = 0; i < 2; i++) {
            // Esta funcion es bloqueante. Aqui no importa mucho no hacerlo realtime
            // ya que el usuario ni siquiera notara la espera de 140ms
            digitalWrite(BUZZER, HIGH);//enciende el buzzer
            delay(1000); //espera 1s
            digitalWrite(BUZZER, LOW); //apaga el buzzer
            delay(500);//espera 500ms
          }
          delay(800); //Evita el empalme de los bips
        }
        else tiempoExposicion = 0; //0 minutos -- hay un valor invalido
        modoDesinfeccion = true;
        interval = 60000; //Carga 60 segundos para la cuenta atras
        previousMillis = millis();
      }
      button_state_machine = User_button.clicks;
    }

    /*** Empezar la cuenta regresiva para encender las lamparas**/
    if (modoDesinfeccion && !desinfeccionEnProceso) { //Solo necesitamos la cuenta regresiva si vamos a encender las lamparas UV
      currentMillis = millis();
      long elapsedTime = currentMillis - previousMillis;
      if (elapsedTime <= 20000) buzzer.Pattern(100, 1000);
      if ((elapsedTime > 20000) && (elapsedTime <= 30000)) buzzer.Pattern(100, 500);
      if ((elapsedTime > 30000) && (elapsedTime <= 40000)) buzzer.Pattern(100, 250);
      if ((elapsedTime > 40000) && (elapsedTime <= 55000)) buzzer.Pattern(100, 125);
      if (elapsedTime > 55000) buzzer.Pattern(100, 0);
      if (currentMillis - previousMillis >= interval) {
        desinfeccionEnProceso = true;
        //UVtimer.setCounter(Horas, Minutos, Segundos, UVtimer.COUNT_DOWN, onComplete);
        UVtimer.setCounter(0, tiempoExposicion, 0, UVtimer.COUNT_DOWN, onComplete);
        UVtimer.run();
      }
    }

    if (desinfeccionEnProceso) { //Si la consigna de encender esta activa
      UVtimer.start(); //Activa la cuenta regresiva
      if (!sensors.pir) { //Si el sensor de presencia no esta activo

        if (pirWasTriggered) { //Despues de la activacion de PIR sensor adevertir que los UV seran encendidos
          pirWasTriggered = false; //Actualiza el ultimo estado del PIR
          digitalWrite(BUZZER, HIGH); //enciende el buzzer por un segundo
          delay(1000);
          UVtimer.start(); //El timer sigue contando desde donde se quedo antes que el PIR se activara
        }
        digitalWrite(LAMPS, HIGH); //Enciende la lampara
        buzzer.Pattern(100, 1000); //haz un beep cada segundo
      }
      else { //Si el sensor PIR detecta a alguien
        digitalWrite(LAMPS, LOW); //Apaga inmediatamente la lampara
        buzzer.Pattern(0, 0); //Apaga el beeper
        pirWasTriggered = true; //Actualiza el ultimo estado del PIR
        UVtimer.pause(); //Pone en pausa el timer
      }
    }
    UVtimer.run(); //Tick para el timer
  }// while (sensors.mode != MAN)

  //Salir del modo automatico
  digitalWrite(LAMPS, LOW); //Al salir del modo automatico se apagan las luces
  desinfeccionEnProceso = false; //Actualiza los trackers de estado
  modoDesinfeccion = false; //Actualiza los trackers de estado
  buzzer.Pattern(0, 0); // y se apaga el buzzer
  buzzer.Update();
  UVtimer.stop();
}//Auto mode fnc

/**** Auxiliares ****/
/*** Read Sensors***/
void readSensors() {
  modesw1_debouncer.update();
  modesw2_debouncer.update();
  pir_debouncer.update();
  shield_debouncer.update();
  deadman_debouncer.update();
  sensors.pir = pir_debouncer.read();
  sensors.shield = !shield_debouncer.read();
  sensors.mode = modesw1_debouncer.read() + 2 * modesw2_debouncer.read();
  sensors.deadman = !deadman_debouncer.read();
}

/** TIMER CALLBACKS **/
/**** Time on Complete***/
void onComplete() {
  Serial.println("Complete!!!");
  desinfeccionEnProceso = false;
  digitalWrite(LAMPS, LOW); //Al salir del modo automatico se apagan las luces
  modoDesinfeccion = false;
  buzzer.Pattern(0, 0); // y se apaga el buzzer
  buzzer.Update();
  UVtimer.stop();
}

/** Print Sensors Para debug**/
void printSensors(void) {
  Serial.print("PIR: ");
  Serial.print(sensors.pir);
  Serial.print(" | SHIELD: ");
  Serial.print(sensors.shield);
  Serial.print(" | MODE: ");
  Serial.print(sensors.mode);
  Serial.print(" | DEADMAN: ");
  Serial.println(sensors.deadman);
}
