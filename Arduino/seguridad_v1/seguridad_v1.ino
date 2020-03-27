

const int endstopper1 =  2; // Endstopper 1
const int onoff =  3; // Endstopper 1


int endstopper1State = 0;
int onoffState = 0;
int lastendstopper = 1;

const int alarm =  12;// Alarm on pin 4

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

const long interval = 6000;   // Set the Time in milliseconds to activate the alarm 1000 = 1 sec

void setup() {
  pinMode(alarm, OUTPUT);
  pinMode(endstopper1, INPUT);
  pinMode(onoff, INPUT);
  Serial.begin(9600);
}

void loop() {
  onoffState = digitalRead(onoff);
  if (onoffState == HIGH) {
    currentMillis = millis();
    Serial.println(currentMillis - previousMillis);
    if (currentMillis - previousMillis >= interval) {
      digitalWrite(alarm, HIGH);
      delay(500);
      digitalWrite(alarm, LOW);
    }
    endstopper1State = digitalRead(endstopper1);


    if (endstopper1State != lastendstopper) {
      if (lastendstopper == 0) {
        lastendstopper = 1;
        previousMillis =  millis();
      } else {
        lastendstopper = 0;
        previousMillis =  millis();
      }
    }
  } else {
    
    delay(50);
    
  }

}
