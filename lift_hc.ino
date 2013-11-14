/* Sensor pins */
#define LEFT_LIGHT_SENSOR A0
#define CENTER_LIGHT_SENSOR A1
#define RIGHT_LIGHT_SENSOR A2
#define PROXIMITY_SENSOR 4

/* Motors PWM pins */
#define LEVITATION_FAN 3
#define LEFT_PROP_FANS 5
#define CENTER_PROP_FAN 9
#define RIGHT_PROP_FANS 10

// Threshold values
#define LIGHT_SENS_THRESHOLD 60
#define NEAR_PEDESTAL_THRESHOLD 9  // inches
#define AT_PEDESTAL_THRESHOLD 3

/* Power percentages */
#define _PERCENT_100 255
#define _PERCENT_90  230
#define _PERCENT_80  204
#define _PERCENT_70  179
#define _PERCENT_60  153
#define _PERCENT_50  128
#define _PERCENT_40  102
#define _PERCENT_30  77
#define _PERCENT_20  51
#define _PERCENT_10  26
#define _PERCENT_0   0

//               HOVERCRAFT VIEW
//          |\                     /|
//          | \                   / |
//          |  \-----------------/  |
//          |   |  *         *  |   |
//          |---|-------*-------|---|
//          |                       |
//          |                       |
//          |                       |
//          |left      LEV     right|
//          |prop      FAN     prop |
//          |fans              fans |
//          |                       |
//          |                       |
//          |          cent         |
//          |          fan          |
//           -----------------------
 
enum State {
  LEFT_DETECTING_LINE = 2, CENTER_DETECTING_LINE = 3, RIGHT_DETECTING_LINE = 5,
  LEFT_AND_CENTER_DETECTING_LINE = 6, LEFT_AND_RIGHT_DETECTING_LINE = 10,
  RIGHT_AND_CENTER_DETECTING_LINE = 15,
  LEFT_CENTER_AND_RIGHT_DETECTING_LINE = 30
};

void setup() {
  Serial.begin(9600);
  setPWMPins();
}

void loop() {
  
  static int previousState = 0;
  int state = checkState();
  
  switch (state) { 
    case CENTER_DETECTING_LINE:
    // center sensor detecting line
    // go forward
    if (isPedestalNear()) {
      // go at slower speed
      levitate(_PERCENT_100);
      controlPropellers(_PERCENT_80, _PERCENT_80, _PERCENT_80);
      if (isTargetReached()) {
        // enable jibboom :)
        enablePayloadSystem();
      }
    } else {
      // go at regular speed
      levitate(_PERCENT_100);
      controlPropellers(_PERCENT_100, _PERCENT_100, _PERCENT_100);
    }
    previousState = state;
    Serial.println("center");
      break;
      
    case LEFT_AND_CENTER_DETECTING_LINE:
    // left + center sensors detecting line
    // rotate CCW slightly
    levitate(_PERCENT_100);
    controlPropellers(_PERCENT_50, _PERCENT_100, _PERCENT_100);
    previousState = state;
    Serial.println("left + center");
      break;
      
    case RIGHT_AND_CENTER_DETECTING_LINE:
    // right + center sensors detecting line
    // rotate CW slightly
    levitate(_PERCENT_100);
    controlPropellers(_PERCENT_100, _PERCENT_100, _PERCENT_50);
    previousState = state;
    Serial.println("right + center");
      break;
      
    case LEFT_DETECTING_LINE:
    // left sensor detecting line
    // a more severe case, rotate CCW cosiderably
    levitate(_PERCENT_100);
    controlPropellers(_PERCENT_20, _PERCENT_50, _PERCENT_100);
    previousState = state;
    Serial.println("left");
      break;
      
    case RIGHT_DETECTING_LINE:
    // right sensor detetcting line
    // a more severe case, rotate CW cosiderably
    levitate(_PERCENT_100);
    controlPropellers(_PERCENT_100, _PERCENT_50, _PERCENT_20);
    previousState = state;
    Serial.println("right");
     break;
      
    case LEFT_AND_RIGHT_DETECTING_LINE:
    // left + right sensors detecting line
    // unlikely to happen, but go forward slowly
    levitate(_PERCENT_100);
    controlPropellers(_PERCENT_80, _PERCENT_80, _PERCENT_80);
    previousState = state;
    Serial.println("left + right");
      break;
      
    case LEFT_CENTER_AND_RIGHT_DETECTING_LINE:
    // left + center + right sensor detecting line
    // less likely to happen, but go forward slowly
    levitate(_PERCENT_100);
    controlPropellers(_PERCENT_80, _PERCENT_80, _PERCENT_80);
    previousState = state;
    Serial.println("left + center + right");
      break;
      
    default:
    // line not detected by any sensor at this state
    if (previousState == 0) {
      // hovercraft hasn't started the course
      // start levitating and go forward slowly
      levitate(_PERCENT_100);
      controlPropellers(_PERCENT_80, _PERCENT_80, _PERCENT_80);
    } else {
      // hovercraft already started the course and is lost
      // so go back to previous state
      // TO BE IMPLEMENTED (after preliminary testing)
    }
    Serial.print("none, previous state:");
    Serial.println(previousState);
      break;
  }
  delay(50);
}

int checkState() {
  int state = 1;
  
  int left = analogRead(LEFT_LIGHT_SENSOR);
  int center = analogRead(CENTER_LIGHT_SENSOR);
  int right = analogRead(RIGHT_LIGHT_SENSOR);
  
  left = map(left, 0, 1023, 0, 255);
  center = map(center, 0, 1023, 0, 255);
  right = map(right, 0, 1023, 0, 255);
  
  Serial.print("L: ");
  Serial.print(left);
  Serial.print("     C: ");
  Serial.print(center);
  Serial.print("     R: ");
  Serial.print(right);
  Serial.println();
  
  // find state
  if (left < LIGHT_SENS_THRESHOLD) state *= LEFT_DETECTING_LINE;
  if (center < LIGHT_SENS_THRESHOLD) state *= CENTER_DETECTING_LINE;
  if (right < LIGHT_SENS_THRESHOLD) state *= RIGHT_DETECTING_LINE;
  
  return state;
}

void levitate(int powLevel) {
  // @param powLevel is a value between 0 - 255
  analogWrite(LEVITATION_FAN, powLevel);
}

void controlPropellers(int leftPowLevel, int centerPowLevel, int rightPowLevel) {
  analogWrite(LEFT_PROP_FANS, leftPowLevel);
  analogWrite(CENTER_PROP_FAN, centerPowLevel);
  analogWrite(RIGHT_PROP_FANS, rightPowLevel);
}

boolean isPedestalNear() {
  return getDistance() < NEAR_PEDESTAL_THRESHOLD;
}

long getDistance() {
  // establish variables for duration of the ping, 
  // and the distance result in inches and centimeters:
  long duration, inches;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(PROXIMITY_SENSOR, OUTPUT);
  digitalWrite(PROXIMITY_SENSOR, LOW);
  delayMicroseconds(2);
  digitalWrite(PROXIMITY_SENSOR, HIGH);
  delayMicroseconds(5);
  digitalWrite(PROXIMITY_SENSOR, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(PROXIMITY_SENSOR, INPUT);
  duration = pulseIn(PROXIMITY_SENSOR, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
  return inches;
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

boolean isTargetReached() {
  return getDistance() < AT_PEDESTAL_THRESHOLD;
}

void enablePayloadSystem() { 
/* # define BIN_ONE 1
 # define BIN_ONE 2
 # define BIN_ONE 3
 # define BIN_ONE 4

 if (isTargetReached = true) {
  delay(500);
  //recall RF sensor bin identification
  int bin = RF_SENSOR_OUTPUT;
  switch (bin) {
   case BIN_ONE:
   //rotate Motor1 to bin 1
   PAYLOAD_ROTATE_MOTOR = 
   delay(1000);
    break;
   
   case BinTwo:
   //rotate Motor1 to bin 2
   PAYLOAD_ROTATE_MOTOR = 
   delay(1000);
    break;
   
   case binThree:
   //rotate Motor1 to bin 3
   PAYLOAD_ROTATE_MOTOR = 
   delay(1000);
    break;
    
   case binFour:
   //rotate Motor1 to bin 4
   PAYLOAD_ROTATE_MOTOR = 
   delay(1000);
    break;
  }
  //drop it
  
  //lift arm */
 }

void setPWMPins() {
  pinMode(LEVITATION_FAN, OUTPUT);
  pinMode(LEFT_PROP_FANS, OUTPUT);
  pinMode(CENTER_PROP_FAN, OUTPUT);
  pinMode(RIGHT_PROP_FANS, OUTPUT);
}
