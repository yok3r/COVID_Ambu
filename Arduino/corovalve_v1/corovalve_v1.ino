

// LIBRARIES //
#define dirPin 2
#define stepPin 3

int led1 = 5;
int led2 = 6;
int endstopper = 7;
int pressure = 8;

int pot1 = A0;
int pot2 = A1;
int pot3 = A2;

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

float speedMin = 1; // MIN motor Speed
float speedMax = 50; // MAX motor Speed
float actualSpeed = 0;

int peet_min = 5; // Value of pressure min to PEET procedure
int presure_max = 50; // Security value to stop pressing



// OTHERS //
// Buttons
const int startbutton = 4;

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
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(endstopper, INPUT);
  pinMode(pressure, INPUT);
}

void loop() {




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
        for (int i = 0; i < ((stepsPerRevolution / j)*actualVolume); i++) { //Divides el Stepsperrevolution por la J que son las veces que quieres revisar las variables.
          // Move the motor
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(2000);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(2000);
        }
        Serial.print("Motor fase: ");
        Serial.println(j);
      }
      state = 2;
      break;

    case 2: // Motor up+
      //for (int i = 0; (i < actualVolume) && (endstopperValue == LOW)); i++) {
      //endstopperValue = analogRead(endstopper);


      digitalWrite(dirPin, LOW); // Set the spinning direction counterclockwise:

      for (int j = 0; j < 10; j++) { //Numero de veces que revisas las variables mientras corre el motor
        // check variables
        for (int i = 0; i < ((stepsPerRevolution / j)*actualVolume); i++) { //Divides el Stepsperrevolution por la J que son las veces que quieres revisar las variables.
          // Move the motor
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(2000);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(2000);
        }
        Serial.print("Motor fase: ");
        Serial.println(j);
      }

      state = 0;
      break;

    case 3: // PEET
      // statements
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
