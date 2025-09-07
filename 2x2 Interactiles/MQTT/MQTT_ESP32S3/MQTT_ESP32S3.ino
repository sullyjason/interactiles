//Code to test the functionality of all sensors and actuators with new CLAW PCB
//working as of 09.09.24
//all pin assignments correct
//callbrated, +- 10g at factor of -210 was achieved using callibration script, but will not work here
//LRA not integrated

#include <HX711.h>
#include <DRV8833.h>
#include <PWMServo.h>

#define DEBUG 0

const int LOADCELL_DOUT_PIN = 18;
const int LOADCELL_SCK_PIN = 19;
long reading = 0;
HX711 scale;

#define Xaxis_pin A3 // Arduino pin connected to the VRx Pin - pin 17
#define Yaxis_pin A2 // Arduino pin connected to the VRy Pin - pin 16
#define SW_pin 15 // Arduino pin connected to the SW Pin
#define LIGHT_SENSOR A6      // select the pin for the LED

DRV8833 driver = DRV8833();
String vibration = "10";

#define SLP_PIN 4

const int inputA1 = 9, inputA2 = 10;


// /******* Voise Coil parametrs ******/

int   patternLength = 500;
int   hapticPattern[ 500 ];
int   patternIndex = 0;

uint32_t timeSinceLoop = 0;
uint32_t timeSinceLoadCell = 0;
uint32_t timeSinceTest = 0; 
uint32_t timeSinceAmpChange = 0;
uint32_t timeSinceLRATest = 0;

uint32_t testLoop = 1000000; // microseconds
uint32_t loadCellPing = 110000; //microseconds. Any less than 90ms or 110000µs and it will not find hx711 every time
float timeBetweenAmpChanges; // Was previously called timeBetweenUpdates 
// uint32_t LRATestInterval = 1000000;
// uint32_t LRAStart = 
// uint32_t LRAStop = 

uint32_t Now = 0;
float deltat = 0.0f;        // integration interval for both filter schemes

bool LED_STATE = 0;

PWMServo myservo;  // create servo object to control a servo
#define SERVO_PIN 5
const int servoOpen = 115;    // when claw is fully open (widest possible position for finger)
const int servoClosed = 165;  // fully closed
float timeBetweenServoMoves = 6000; //microseconds
uint32_t timeSinceServoMove = 0;
int servoPos;
int joystickPos;
const int joystickOpen = 250; //when the joystick is moved left far enough, it will reach this value. Compare joystick analogread value to this.
const int joystickClose = 780; // ditto but when moved right



///////////////////BLINKING LED//////////////////////

//to show the device is runnning

void blinkLED() {
  if (LED_STATE) {
    digitalWrite (LED_BUILTIN, LOW);
    LED_STATE = 0;
  } else {
    digitalWrite (LED_BUILTIN, HIGH);
    LED_STATE = 1;
  }
}


///////////////////JOYSTICK//////////////////////

void setupJoystick() {
  pinMode(SW_pin, INPUT_PULLUP);
  pinMode(Xaxis_pin, INPUT);
  pinMode(Yaxis_pin, INPUT);
}

void readJoystick(){
  Serial.print("X-axis: ");
  Serial.print(analogRead(Xaxis_pin));
  Serial.print(" | ");
  Serial.print("Y-axis: ");
  Serial.print(analogRead(Yaxis_pin));
  Serial.print(" | ");
  Serial.print("Switch:  ");
  Serial.print(digitalRead(SW_pin));
  Serial.print(" | ");
}

void getJoyStickVal() {
    joystickPos = analogRead(Xaxis_pin);
}

//////////////////LOAD CELL///////////////////////

void setupLoadCell() {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(-2100);
}

void readLoadCell() {
  if (scale.is_ready()) {
    reading = scale.read();
  } else {
    reading = 0;
  }
}

void printLoadCell() {
    Serial.print("Load cell: ");
  if (reading != 0) {
    Serial.print(scale.get_units(), 453.592);  } else {
    Serial.print("HX711 not found.");
  }
   Serial.print(" | ");
}


//////////////////LIGHT SENSOR//////////////////////

void readLightSensor() {
  // read the value from the sensor:  // turn the ledPin on
  int sensorValue = analogRead(LIGHT_SENSOR);
  Serial.print("Light sensor: ");
  Serial.print(sensorValue);
  Serial.print(" | ");
}

///////////////////////////////////////

void setupServo() {
  myservo.attach(SERVO_PIN);  // attaches the servo on pin 9 to the servo object
  servoPos = 155;
}

void servoMove() {
  if ((joystickPos >= joystickClose) && (servoPos < servoClosed)){
    servoPos += 1;
    myservo.write(servoPos);
  }
  if ((joystickPos <= joystickOpen) && (servoPos > servoOpen)){
    servoPos -= 1;
    myservo.write(servoPos);
  }  
  timeSinceServoMove = 0;
}

//////////////////////////////////////////

/////////////////LRA///////////////////////////

void LRATest() {
  driver.motorBForward(700);
  driver.motorBStop();
  // timeSinceLRATest = 0;
}


///////////////VOICE COIL////////////////////////

uint32_t lastUpdate = 0, firstUpdate = 0; // used to calculate integration interval
float updateFrequency = 600.0f;

void setupCoil() {
  analogWriteResolution(10);

  driver.attachMotorA(inputA1, inputA2); // Texturing Voice Coil
  //driver.attachMotorB(inputB1, inputB2); // Main Body Vibration
  driver.motorAStop();
  driver.motorBStop();

  digitalWrite(SLP_PIN, HIGH);

  for ( int i = 0; i < patternLength; ++i ){
    hapticPattern[i] = 0;
  }

  timeBetweenAmpChanges = 1.0f / updateFrequency;
  DoPatternCommand(vibration);
}


void DoPatternCommand(String val) {
  
  int amp = val.toInt();

  if(amp > 400) {
    amp = 400;
  }
  if(val < 0) {
    amp = 0;
  }
  for ( int i = 0; i < patternLength; ++i ) {
    hapticPattern[i] = random(-amp, amp);
  }
}

void playPattern() {
    if (timeBetweenAmpChanges < 1.0) {
      if (hapticPattern[ patternIndex ]>=0){
        analogWriteFrequency(3, 21000);
        analogWriteFrequency(4, 21000);
        driver.motorAForward(hapticPattern[ patternIndex ]);
      }else{
        analogWriteFrequency(3, 21000);
        analogWriteFrequency(4, 21000);
        driver.motorAReverse( -hapticPattern[ patternIndex ] );
      }
    }

    else {
      driver.motorAStop();
    }

    patternIndex += 1;
    if (patternIndex>=patternLength) {
      patternIndex = 0;
    }
    timeSinceAmpChange = 0.0f;
}


void setup() {
  delay(2000);
  Serial.begin(2000000); 
  pinMode(SLP_PIN, OUTPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  setupLoadCell();
  setupJoystick();
  setupServo();
  setupCoil();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LIGHT_SENSOR, INPUT);
  //LRATest();

}


void loop() {

  blinkLED();
  Now = micros();

  deltat = (Now - timeSinceLoop);
  timeSinceLoop = Now;
  timeSinceLoadCell += deltat;
  timeSinceTest += deltat;
  timeSinceLRATest += deltat;
  timeSinceAmpChange += deltat;  
  timeSinceServoMove += deltat;

  #if DEBUG
  Serial.print(deltat);
  Serial.print(" / ");
  Serial.print(timeSinceLoadCell);
  Serial.print(" / ");
  Serial.println(timeSinceTest);
  #endif

  if ((timeSinceLoadCell) >= loadCellPing){
    readLoadCell();
    timeSinceLoadCell = 0;
    }

  if ((timeSinceTest) >= testLoop){
    readJoystick();
    readLightSensor();
    printLoadCell();
    Serial.println(" ");
    timeSinceTest = 0;
    }

  // if (timeSinceLRATest >= LRATestInterval) {
  //   LRATest();
  // }

  if (timeSinceAmpChange >= timeBetweenAmpChanges){
    playPattern();
    }
  
  if (timeSinceServoMove >= timeBetweenServoMoves){
    getJoyStickVal();
    servoMove();
  }
}