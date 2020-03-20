
// LIBRARIES //
#define dirPin 2
#define stepPin 3
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//DIGITALS//
const int endstopper = 5; // Pin del endstopper
const int onoff = 4; // Interruptor ON / OFF
const int modeselector = 6; // Pin del selector de modo Volumen / Presion
int modeselectorState = 0; // Estado del modo*
const float SensorOffset = 41.0; // Valor para ajustar el sensor de presión a cmH2o

//ANALOGS//
const int pot1 = A0;  // Entrada potenciometro 1
const int pot2 = A1; // Entrada potenciometro 1
const int pot3 = A2; // Entrada potenciometro 1
int peep = A3; // Entrada del sensor de presions
//A4 - LCD Screen SDA
//A5 - LCD Screen SCL

// Variables para guardar los valores:
int valuepot1 = 0; // Tidal Volume (Liters)
int valuepot2 = 0; // Breaths per minute
int valuepot3 = 0; // Speed


// PROTOTYPE ADJUSTS //
#define stepsPerRevolution 1  // change this to fit the number of steps per revolution

int distanciaPresionMax = 1000; // Distancia maxima en la ida por presion
int ciclosMoverMotor = 5; // Los ciclos que hace cada vez que se mueve el motor

//  Valores de Retorno
int maxup = 100; //Maxima distancia del motor al volver
int velRetorno = 800; // Velocidad de retorno del motor (menos es mas rapido)


// Ajustes de presion maxima y minima del sensor de presion (Thresholds)
int threshold_presure_min = 20; // Value of pressure min to PEEP procedure
int threshold_presure_max = 600; // Security value to stop pressing
float peepAdjustMin = 0;
float peepAdjustMax = 100;



// INITIAL VARIABLES  //
//Per volumen
int volumeMin = 50; //Initial air volume per cicle
int volumeMax = 500;
float actualVolume = 0;
//Per presure
int presureMin = 0; //Initial air volume per cicle
int presureMax = 700;
float actualPresure = 0;

int ciclesMin = 4; // MIN Cicles per minute
int ciclesMax = 30; // MAX Cicles per minute
float actualCicles = 0;

float speedMin = 5000; // MIN motor Speed (More is slower)
float speedMax = 300; // MAX motor Speed (Less is more fast)
float actualSpeed = 0;


float actualpeep = 0;
float valuepeep = 0;


int peepPresure = 10; // Variable to press the peep


// OTHERS //
// Buttons
String impresion = "";
const int alarm = 12;
int stateAlarm = 0;

int onoffState = 0;
int state = 5;
int stepCount = 0; // number of steps the motor has taken
int startState = 0; //Define the state, 1: start 0: stop to prepare the variables before starting
unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
unsigned long lastMillis = 0;    // stores the value of millis() in each iteration of loop()
unsigned long ieMillisStart = 0;
unsigned long ieMillisEnd = 0;
float delaynow = 0;
int pressureValue = 0; // Keep the pressure value
int endstopperValue = 0;

float loops = 10;


void setup() {
  Serial.begin(9600);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(endstopper, INPUT);
  pinMode(onoff, INPUT);
  pinMode(alarm, OUTPUT);
  pinMode(modeselector, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  currentMillis = millis();
  lastMillis = millis();
}

void loop() {

  // Comprueba las variables y la presion (checkPesure)
  // NOTA: He puesto el check variables dentro de los modos porque hay modos que no necesitan analizar las variables y asi van mas rapidos
  //checkVariables();
  //checkPesure();

  // Mira si se tiene que activar la alarma
  if (stateAlarm == HIGH) {
    digitalWrite(alarm, HIGH);
    delay(100);
    digitalWrite(alarm, LOW);
    stateAlarm = 0;
  }

  // Coge los milisegundos actuales para hacer los calculos de cuanto tarda un ciclo
  currentMillis = millis();   // capture the latest value of millis()
  Serial.print("Estado: ");
  Serial.println(state);

  // Si el switch ON/OFF esta encendido se va al estado 5 y se queda en bucle, si esta apagado inicia el programa
  if (onoffState == HIGH) {
    if (state == 5) { // Esto esta asi para que no se ponga state=0 en cada iteracion
      state = 0; // Start
      Serial.println("Start");
    }
  } else {
    state = 5; // Stop
    Serial.println("Stop");
  }

  switch (state) {
    case 0: // STANDBY - Se espera a que toque hacer otro ciclo
      ////  WAIT FOR NEXT CYCLE  ////
      //Serial.println(currentMillis - lastMillis);

      checkVariables(); // Revisamos las variables dentro de cada estado, ya que hay estados que no necesitan las variables y asi reducimos el delay

      if ( currentMillis - lastMillis >= (60000 / actualCicles)) { // Calcula si ha pasado el tiempo necesario - 60000/actualCicles te dice cuanto tiene que durar un ciclo
        lastMillis = millis();
        if (modeselectorState == HIGH) {//Mira en que modo esta el selector de modo y te lleva a uno o otro
          state = 1; // Modo volumen
          Serial.println("Ir al modo VOlumen");
        } else {
          state = 4; // Modo presion
          Serial.println("Ir al modo presion");
        }

      } else { // Si no se ha llegado a los milisegundos para empezar el ciclo mira si se han pasado los valores de presion max o min, si se han pasado se activa la alarma
        //Serial.println("Waiting");
        if (valuepeep <= threshold_presure_min) {
          stateAlarm = 1;
          state = 3;
          break;
        } else if (valuepeep >= threshold_presure_max) {
          stateAlarm = 1;
          state = 2;
          break;
        }
        state = 0;
        break;
      }
      break;

    case 1: // MODO VOLUMEN

      checkVariables();

      Serial.println("Modo 1");
      ieMillisStart = millis();
      digitalWrite(dirPin, HIGH); // Set the spinning direction clockwise:
      for (int i = 0; i < ((stepsPerRevolution)*actualVolume) ; i++) {
        checkPesure();
        if (valuepeep >= threshold_presure_max) {
          state = 2;
          break;
        }
        for (int i = 0; i < ciclosMoverMotor; i++) {
          // Move the motor
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(actualSpeed);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(actualSpeed);
        }
      }
      ieMillisEnd = millis();
      state = 2;
      break;


    case 2: // Motor up+
      //for (int i = 0; (i < actualVolume) && (endstopperValue == LOW)); i++) {

      checkVariables();

      Serial.println("State 2");
      digitalWrite(dirPin, LOW);

      for (int j = 0; j < maxup; j++) { //Numero de veces que revisas las variables mientras corre el motor hasta MAXUP que es el valor maximo que puede recorrer
        checkPesure();

        // Mira si el valor de presion es inferior al Minimo
        if (valuepeep <= threshold_presure_min) {
          state = 3;
          break;
        }

        // Mira si se ha apretado el endstopper
        endstopperValue = digitalRead(endstopper);
        if (endstopperValue == 0) {
          state = 0;
          break;
        }

        // Sino sigue corriendo
        for (int i = 0; i < ciclosMoverMotor; i++) {
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(velRetorno);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(velRetorno);
        }

      }

      state = 0;
      break;

    case 3: // PEEP
      Serial.print("PEEP MODE");

      digitalWrite(dirPin, HIGH); // Set the spinning direction clockwise:

      // Aprieta para dar presión
      for (int i = 0; i < ((stepsPerRevolution) * 50) ; i++) {
        // Move the motor
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(actualSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(actualSpeed);
      }
      state = 2;
      break;

    case 4: // MODO PRESION - De momento no

      /*
            digitalWrite(dirPin, HIGH); // Set the spinning direction clockwise:
            Serial.print("State 4");
            for (int i = 0; i < (distanciaPresionMax) ; i++) {
              checkPesure();
              if (valuepeep >= actualPresure) {
                state = 2;
                break;
              }
              for (int i = 0; i < ((stepsPerRevolution)*peepPresure) ; i++) { //Divides el Stepsperrevolution por la J que son las veces que quieres revisar las variables.
                // Move the motor
                digitalWrite(stepPin, HIGH);
                delayMicroseconds(actualSpeed);
                digitalWrite(stepPin, LOW);
                delayMicroseconds(actualSpeed);
              }
            }

            ieMillisEnd = millis();
            state = 2;

      */

      break;

    case 5: // En este estado no hace nada, es la pausa
      checkVariables();
      checkPesure();
      delay(5);
      break;
  }
}

void checkVariables() {

  modeselectorState  = digitalRead(modeselector);
  onoffState  = digitalRead(onoff);

  // Calculamos el ratio de los ciclos
  float ieMillisRatio = (60000 / actualCicles) / (ieMillisEnd - ieMillisStart);
  //                     minutos / ciclos       inicio del modo    final del modo


  // Read the value of 3 parameters
  valuepot1 = analogRead(pot1); // Volumen - Presion
  valuepot2 = analogRead(pot2); // Ciclos
  valuepot3 = analogRead(pot3); // Velocidad
  endstopperValue = digitalRead(endstopper);


  // Mapeamos los valores de los potenciometros a los ajustes min y max de cada uno
  actualVolume = map(valuepot1, 0, 1024, volumeMin, volumeMax);
  actualPresure = map(valuepot1, 0, 1024, presureMin, presureMax);

  actualCicles = map(valuepot2, 0, 1024, ciclesMin, ciclesMax);
  actualSpeed = map(valuepot3, 0, 1024, speedMin, speedMax);


  // Primera linea de la pantalla LCD
  lcd.setCursor(0, 0);

  if (modeselectorState == HIGH) { //Si esta selecionado el modo presion
    impresion = ("VT " + String((int)actualVolume) + "mL "  +  String((int) actualCicles) + "/min   ");
  } else {
    impresion = ("P " + String((int)actualPresure) + "cmH2o "  +  String((int) actualCicles) + "/min   ");
  }

  //Serial.print(impresion);
  lcd.print(impresion);
  lcd.setCursor (0, 1);

  if (modeselectorState == HIGH) {//Si esta selecionado el modo presion
    impresion = ("1:" + String(ieMillisRatio) + " " + String((int)actualpeep) + "cmH2o  ");
  } else {
    impresion = ("1:" + String(ieMillisRatio) + "  " + String((int)actualpeep) + "mL   ");
  }

  //Serial.print(impresion);
  lcd.print(impresion);
  lcd.display();




}

void checkPesure() {
  valuepeep = analogRead(peep);
  //Serial.print(" Peep: ");
  // Serial.println(valuepeep);

  ///   //actualpeep = (valuepeep - SensorOffset - 512.0) / 10.0;
  actualpeep =  valuepeep;
  //valuepeep = 45;

}
//actualpeep = map(valuepeep, 0, 1000, peepAdjustMin, peepAdjustMax);
