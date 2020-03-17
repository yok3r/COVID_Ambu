

// LIBRARIES //
#define dirPin 2
#define stepPin 3

const int endstopper = 5;
const int pressure = 8;

const int pot1 = A0;
const int pot2 = A1;
const int pot3 = A2;

int valuepot1 = 0; // Tidal Volume (Liters)
int valuepot2 = 0; // Breaths per minute
int valuepot3 = 0; // Inale to exhale time ratio

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

float peepAdjustMin = 10;
float peepAdjustMax = 5000;
float actualpeep = 0;
float valuepeep = 0;
int peep_min = 5; // Value of pressure min to PEEP procedure
int presure_max = 50; // Security value to stop pressing



// OTHERS //
// Buttons
const int startbutton = 4;

const int alarm = 12;
int peep = A3;
int state = 4;
int stepCount = 0; // number of steps the motor has taken
int startState = 0; //Define the state, 1: start 0: stop to prepare the variables before starting
unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
float delaynow = 0;
int pressureValue = 0; // Keep the pressure value
int endstopperValue = 0;

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  Serial.begin(9600);
  pinMode(startbutton, INPUT);
  pinMode(endstopper, INPUT);
  pinMode(pressure, INPUT);
  pinMode(alarm, OUTPUT);
}

void loop() {

  checkVariables();
  checkPeep();

  currentMillis = millis();   // capture the latest value of millis()
  // Define the variables of new loop
  //Speed = fix * desired duration

  //Serial.println(currentMillis);

  if (startState == HIGH) { // Check to start
    state = 1;
    startState = 0;
  } else { // Waiting the button to start
    startState = digitalRead(startbutton);
    //Serial.println(startState);
  }

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

      for (int j = 0; j < 10; j++) { //Numero de veces que revisas las variables mientras corre el motor
        // check variables
        checkVariables();
        checkPeep();
        for (int i = 0; i < ((stepsPerRevolution / j)*actualVolume); i++) { //Divides el Stepsperrevolution por la J que son las veces que quieres revisar las variables.
          // Move the motor
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(actualSpeed);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(actualSpeed);
        }
      }
      state = 2;
      break;

    case 2: // Motor up+
      //for (int i = 0; (i < actualVolume) && (endstopperValue == LOW)); i++) {
      //endstopperValue = analogRead(endstopper);

      digitalWrite(dirPin, LOW); // Set the spinning direction counterclockwise:

      //while (endstopperValue == 1) { //Numero de veces que revisas las variables mientras corre el motor
      for (int i = 0; i < 20; i++) {
        // check variables
        checkVariables();
        checkPeep();
        if (actualpeep > 20) {
          state = 3;
          break;
        }
        for (int i = 0; i < stepsPerRevolution * 25; i++) { //Ajustar el 25
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(actualSpeed);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(actualSpeed);
        }
      }

      state = 0;
      break;

    case 3: // PEEP

      Serial.print("PEEP MODE");

      digitalWrite(dirPin, HIGH);
      for (int i = 0; i < ((stepsPerRevolution)*actualVolume / 10); i++) {
        // Move the motor

        digitalWrite(stepPin, HIGH);
        delayMicroseconds(actualSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(actualSpeed);
      }
      break;

    case 4: // PAUSED
      break;
  }
}

void checkVariables() {

  // Read the value of 3 parameters
  valuepot1 = analogRead(pot1);
  valuepot2 = analogRead(pot2);
  valuepot3 = analogRead(pot3);
  endstopperValue = digitalRead(endstopper);

  actualVolume = map(valuepot1, 20, 1000, volumeMin, volumeMax);
  actualCicles = map(valuepot2, 20, 1000, ciclesMin, ciclesMax);
  actualSpeed = map(valuepot3, 20, 1000, speedMin, speedMax);

  Serial.print("Volume: ");
  Serial.print(actualVolume);
  Serial.print(" |  Cicles: ");
  Serial.print(actualCicles);
  Serial.print(" |  Speed: ");
  Serial.print(actualSpeed);
  Serial.print(" |  Endstopper: ");
  Serial.println(endstopperValue);
}

void checkPeep() {
  valuepeep = analogRead(peep);
  actualpeep = map(valuepeep, 0, 1000, peepAdjustMin, peepAdjustMax);

  Serial.print("Peep: ");
  Serial.println(actualpeep);

}
