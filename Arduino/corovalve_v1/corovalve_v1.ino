

// LIBRARIES //
#define dirPin 2
#define stepPin 3
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int endstopper = 5; // Pin del endstopper
const int modeselector = 6; // Pin del selector de modo Volumen / Presion
int modeselectorState = 0; // Estado del modo

//ANALOGS//
const int pot1 = A0;
const int pot2 = A1;
const int pot3 = A2;
int peep = A3;
//A4 - LCD Screen SDA
//A5 - LCD Screen SCL

//DIGITALS//

int valuepot1 = 0; // Tidal Volume (Liters)
int valuepot2 = 0; // Breaths per minute
int valuepot3 = 0; // Speed

// PROTOTYPE ADJUSTS //
#define stepsPerRevolution 1  // change this to fit the number of steps per revolution

// INITIAL VARIABLES  //
int volumeMin = 50; //Initial air volume per cicle
int volumeMax = 500;
float actualVolume = 0;

int ciclesMin = 10; // MIN Cicles per minute
int ciclesMax = 70; // MAX Cicles per minute
float actualCicles = 0;

float speedMin = 5000; // MIN motor Speed (More is slower)
float speedMax = 300; // MAX motor Speed (Less is more fast)
float actualSpeed = 0;

float peepAdjustMin = 0;
float peepAdjustMax = 100;
float actualpeep = 0;
float valuepeep = 0;
int TresholdPeep = 30; // Treshold peep
int peep_min = 5; // Value of pressure min to PEEP procedure
int presure_max = 50; // Security value to stop pressing
int peepPresure = 10; // Variable to press the peep


// OTHERS //
// Buttons
String impresion = "";
const int alarm = 12;


int state = 0;
int stepCount = 0; // number of steps the motor has taken
int startState = 0; //Define the state, 1: start 0: stop to prepare the variables before starting
unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
float delaynow = 0;
int pressureValue = 0; // Keep the pressure value
int endstopperValue = 0;

float loops = 10;
int maxup = 30; //Max up the Z

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  Serial.begin(9600);

  pinMode(endstopper, INPUT);
  pinMode(alarm, OUTPUT);
  pinMode(modeselector, INPUT);


  lcd.init();
  lcd.backlight();
  lcd.clear();

}

void loop() {

  checkVariables();
  checkPeep();

  currentMillis = millis();   // capture the latest value of millis()
  Serial.println(state);

  switch (state) {
    case 0: // Standby
      ////  WAIT FOR NEXT CYCLE  ////
      //delaynow = (60000/cicles)-(cicles*2*(speedServo*volume)); // REPASAR

      delaynow = (60000 / actualCicles); // Basico para ir tirando
      Serial.println("Waiting");
      delay(delaynow);
      state = 1;
      break;

    case 1: // Motor down
      digitalWrite(dirPin, HIGH); // Set the spinning direction clockwise:
      for (int i = 0; i < ((stepsPerRevolution)*actualVolume) ; i++) { //Divides el Stepsperrevolution por la J que son las veces que quieres revisar las variables.
        // Move the motor
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(actualSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(actualSpeed);
      }
      state = 2;
      break;


    case 2: // Motor up+
      //for (int i = 0; (i < actualVolume) && (endstopperValue == LOW)); i++) {
      
      Serial.print("State2");
      digitalWrite(dirPin, LOW);
      state = 0; // Next state

      for (int j = 0; j < maxup; j++) { //Numero de veces que revisas las variables mientras corre el motor
        checkPeep();
        if (valuepeep >= TresholdPeep) {
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
          delayMicroseconds(actualSpeed);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(actualSpeed);
        }
      }
      break;

    case 3: // PEEP
      Serial.print("PEEP MODE");
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

    case 4: // PAUSED
      delay(20);
      break;
  }
}

void checkVariables() {


  modeselectorState  = digitalRead(modeselector);

  // Read the value of 3 parameters
  valuepot1 = analogRead(pot1); // Volumen - Presion
  valuepot2 = analogRead(pot2); // Ciclos
  valuepot3 = analogRead(pot3); // Velocidad
  endstopperValue = digitalRead(endstopper);

  actualVolume = map(valuepot1, 20, 1000, volumeMin, volumeMax);
  actualCicles = map(valuepot2, 20, 1000, ciclesMin, ciclesMax);
  actualSpeed = map(valuepot3, 20, 1000, speedMin, speedMax);

  lcd.setCursor(0, 0);
  int actualVolumePrint = (int) actualVolume;
  int actualCiclesPrint = (int) actualCicles;
  int actualSpeedPrint = (int) actualSpeed;

  if (modeselectorState == 0) { //Si esta selecionado el modo presion
    impresion = ("  P " + String(actualVolumePrint) + " FR "  +  String(actualCiclesPrint));
  } else {
    impresion = (" VT " + String(actualVolumePrint) + " FR "  +  String(actualCiclesPrint));
  }

  Serial.print(impresion);
  lcd.print(impresion);
  lcd.setCursor (0, 1);

  if (modeselectorState == 0) {//Si esta selecionado el modo presion
    impresion = ("I:E " + String(actualSpeedPrint) + " VT  ");
  } else {
    impresion = ("I:E " + String(actualSpeedPrint) + " P  ");
  }

  Serial.print(impresion); 
  lcd.print(impresion);
  lcd.display();

  Serial.print("  Endstopper: ");
  Serial.print(endstopperValue);

}

void checkPeep() {
  valuepeep = analogRead(peep);
  Serial.print(" Peep: ");
  Serial.println(valuepeep);

}
//actualpeep = map(valuepeep, 0, 1000, peepAdjustMin, peepAdjustMax);
