
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
const float SensorOffset = 41.0;

//ANALOGS//
const int pot1 = A0;
const int pot2 = A1;
const int pot3 = A2;
int peep = A3;
//A4 - LCD Screen SDA
//A5 - LCD Screen SCL



int valuepot1 = 0; // Tidal Volume (Liters)
int valuepot2 = 0; // Breaths per minute
int valuepot3 = 0; // Speed

// PROTOTYPE ADJUSTS //
#define stepsPerRevolution 1  // change this to fit the number of steps per revolution
int velRetorno = 1000; // Velocidad de retorno del motor
int distanciaIdaMax = 50; // Distancia maxima en la ida por presion
int TresholdPeep = 36; // Treshold peep


// INITIAL VARIABLES  //
int volumeMin = 50; //Initial air volume per cicle
int volumeMax = 500;
float actualVolume = 0;

int ciclesMin = 4; // MIN Cicles per minute
int ciclesMax = 30; // MAX Cicles per minute
float actualCicles = 0;

float speedMin = 5000; // MIN motor Speed (More is slower)
float speedMax = 300; // MAX motor Speed (Less is more fast)
float actualSpeed = 0;

float peepAdjustMin = 0;
float peepAdjustMax = 100;
float actualpeep = 0;
float valuepeep = 0;

int peep_min = 5; // Value of pressure min to PEEP procedure
int presure_max = 50; // Security value to stop pressing
int peepPresure = 10; // Variable to press the peep


// OTHERS //
// Buttons
String impresion = "";
const int alarm = 12;

int onoffState = 0;
int state = 0;
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
int maxup = 30; //Max up the Z

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

  checkVariables();
  checkPeep();

  currentMillis = millis();   // capture the latest value of millis()
  Serial.println(state);

  if (onoffState == HIGH) {
    if (state == 5) {
      state = 0; // Start
    }
  } else {
    state = 5; // Stop
  }

  switch (state) {
    case 0: // Standby
      ////  WAIT FOR NEXT CYCLE  ////
      //delaynow = (60000/cicles)-(cicles*2*(speedServo*volume)); // REPASAR
      //Serial.println(currentMillis - lastMillis);

      if ( currentMillis - lastMillis >= (60000 / actualCicles)) {
        lastMillis = millis();
        if (modeselectorState == HIGH) {//Si esta selecionado el modo presion...
          state = 1; // Modo volumen
        } else {
          state = 4; // Modo presion
        }

      } else {
        //Serial.println("Waiting");
        checkVariables();
        checkPeep();
        if (valuepeep <= TresholdPeep) {
          state = 3;
          break;
        }
        state = 0;
        break;
      }


    case 1: // MODO VOLUMEN
    Serial.println(state);
      ieMillisStart = millis();
      digitalWrite(dirPin, HIGH); // Set the spinning direction clockwise:
      for (int i = 0; i < ((stepsPerRevolution)*actualVolume) ; i++) { //Divides el Stepsperrevolution por la J que son las veces que quieres revisar las variables.
        // Move the motor
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(actualSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(actualSpeed);
      }
      ieMillisEnd = millis();
      state = 2;
      break;


    case 2: // Motor up+
      //for (int i = 0; (i < actualVolume) && (endstopperValue == LOW)); i++) {
      checkVariables();
      Serial.print("State2");
      digitalWrite(dirPin, LOW);
      state = 0; // Next state

      for (int j = 0; j < maxup; j++) { //Numero de veces que revisas las variables mientras corre el motor
        checkPeep();
        if (valuepeep <= TresholdPeep) {
          state = 3;
          break;
        }
        endstopperValue = digitalRead(endstopper);
        if (endstopperValue == 0) { // Para si toca el stopper
          state = 0;
          break;
        }
        for (int i = 0; i < ((stepsPerRevolution)*actualVolume) / loops; i++) {
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(velRetorno);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(velRetorno);
        }
      }
      break;

    case 3: // PEEP
      //Serial.print("PEEP MODE");
      digitalWrite(dirPin, HIGH); // Set the spinning direction clockwise:

      for (int i = 0; i < ((stepsPerRevolution)*peepPresure) ; i++) { //Divides el Stepsperrevolution por la J que son las veces que quieres revisar las variables.
        // Move the motor
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(actualSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(actualSpeed);
      }
      state = 2;
      break;

    case 4: // MODO PRESION
      for (int j = 0; j < distanciaIdaMax; j++) {
        checkPeep();

        for (int i = 0; i < ((stepsPerRevolution)*actualVolume) / loops; i++) {
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(velRetorno);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(velRetorno);
        }
      }
      break;

    case 5:
      delay(5);
      break;
  }
}

void checkVariables() {

  modeselectorState  = digitalRead(modeselector);
  onoffState  = digitalRead(onoff);


  //Serial.print(" Onoff: ");
  //Serial.println(onoffState);

  // Read the value of 3 parameters
  valuepot1 = analogRead(pot1); // Volumen - Presion
  valuepot2 = analogRead(pot2); // Ciclos
  valuepot3 = analogRead(pot3); // Velocidad
  endstopperValue = digitalRead(endstopper);

  actualVolume = map(valuepot1, 0, 1024, volumeMin, volumeMax);
  actualCicles = map(valuepot2, 0, 1024, ciclesMin, ciclesMax);
  actualSpeed = map(valuepot3, 0, 1024, speedMin, speedMax);



  lcd.setCursor(0, 0);
  int actualVolumePrint = (int) actualVolume;
  int actualCiclesPrint = (int) actualCicles;
  int actualSpeedPrint = (int) actualSpeed;

  float ieMillisRatio = (60000 / actualCicles) / (ieMillisEnd - ieMillisStart);

  //Serial.print("IE Ratio:");
  //Serial.println(ieMillisRatio);
  if (modeselectorState == 0) { //Si esta selecionado el modo presion
    impresion = ("P " + String(actualVolumePrint) + "cmH2o "  +  String(actualCiclesPrint) + "/min   ");
  } else {
    impresion = ("VT " + String(actualVolumePrint) + "mL "  +  String(actualCiclesPrint) + "/min   ");
  }

  //Serial.print(impresion);
  lcd.print(impresion);
  lcd.setCursor (0, 1);



  if (modeselectorState == 0) {//Si esta selecionado el modo presion
    impresion = ("1:" + String(ieMillisRatio) + "  " + String((int)actualpeep) + "mL   ");
  } else {
    impresion = ("1:" + String(ieMillisRatio) + " " + String((int)actualpeep) + "cmH2o  ");
  }

  //Serial.print(impresion);
  lcd.print(impresion);
  lcd.display();

  // Serial.print("  Endstopper: ");
  // Serial.print(endstopperValue);

}

void checkPeep() {
  valuepeep = analogRead(peep);
  //Serial.print(" Peep: ");
  //Serial.println(valuepeep);

  ///   //actualpeep = (valuepeep - SensorOffset - 512.0) / 10.0;
  actualpeep = 50;

}
//actualpeep = map(valuepeep, 0, 1000, peepAdjustMin, peepAdjustMax);
