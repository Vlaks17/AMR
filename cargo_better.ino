#include <QTRSensors.h>

#define NUM_SENSORS 8
QTRSensors qtr;
uint16_t sensorValues[NUM_SENSORS];

// way
const char* Way[] = {"left","3","left","1","left","1"};
bool up[] = {true,false,false};
// cnt
int cntL = 0, lastL = 1;
int cntR = 0, lastR = 0;
#define test 50
// motors
#define PWM_A 11
#define PWM_B 3
#define IN1 40
#define IN2 39
#define IN3 38
#define IN4 37

// sensors
#define sensorL 22
#define sensorR 23

// PID
float lastError = 0.0;
float kp = 0.05;
float kd = 0.15;

int m1Speed = 0;
int m2Speed = 0;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(PWM_A, OUTPUT); pinMode(PWM_B, OUTPUT);

  pinMode(sensorL, INPUT);
  pinMode(sensorR, INPUT);

  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){2,4,5,6,7,8,9,10}, NUM_SENSORS);

  delay(1000);
  for (int i = 0; i < 300; i++) {
    qtr.calibrate();
    delay(2);
  }
  cntL = 0;
  cntR = 0;
}

// ---------------- LINE FOLLOW ----------------

void line(){
  uint16_t line_position = qtr.readLineBlack(sensorValues);

  digitalWrite(IN1,LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH);

  float error = line_position - 3500.0;
  if(abs(error)<200) error=0;

  float d = (error - lastError)*0.7;
  float PV = kp*error + kd*d;

  lastError = error;
  PV = constrain(PV,-40,40);

  int baseSpeed = (abs(error)<300)?140:120;
  if(error==0) baseSpeed=170;

  m1Speed = constrain(baseSpeed-PV,0,255);
  m2Speed = constrain(baseSpeed+PV,0,255);

  analogWrite(PWM_A,m1Speed);
  analogWrite(PWM_B,m2Speed);
}

// ---------------- cnt ----------------


int CNT_L() {
  int now = digitalRead(sensorL);
  if(now == 0 && lastL == 1) cntL++;
  lastL = now;
  return cntL;
}


int CNT_R() {
  int now = !digitalRead(sensorR);  
  if(now == 0 && lastR == 1) cntR++;
  lastR = now;
  return cntR;
}

// ---------------- turn ----------------
void turnLeft(){
  analogWrite(PWM_A,150);
  analogWrite(PWM_B,150);

  digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
  digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);

  delay(600);
}

void turnRight(){
  analogWrite(PWM_A,150);
  analogWrite(PWM_B,150);

  digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH);

  delay(600);
}

// ---------------- move ----------------
void move_function(int steps, const char* dir, bool servo){

  while(true){
    line();

    if(strcmp(dir,"left")==0 && CNT_L()>=steps) break;
    if(strcmp(dir,"right")==0 && CNT_R()>=steps) break;
  }

  if(strcmp(dir,"left")==0) turnLeft();
  if(strcmp(dir,"right")==0) turnRight();
  while(true){
    line();

    int L = digitalRead(sensorL);
    int R = !digitalRead(sensorR); 

    if(L==0 && R==0){
      analogWrite(PWM_A,HIGH);
      analogWrite(PWM_B,HIGH);

      digitalWrite(IN1,HIGH); digitalWrite(IN2,HIGH);
      digitalWrite(IN3,HIGH); digitalWrite(IN4,HIGH);
      if(servo){
        digitalWrite(test,HIGH);
      }else
        digitalWrite(test,LOW);
      delay(1000);
      break;
    }
  }
  cntL = 0;
  cntR = 0;

}
// ---------------- LOOP ----------------
void loop() {
cntL = 0;
cntR = 0;
  for(int i=0; i<3; i++){
    cntL = 0;
    cntR = 0;
    int steps = atoi(Way[i*2+1]);
    const char* dir = Way[i*2];
    bool servo = up[i];
    move_function(steps, dir,servo);

    
    while(true){
      line();

      int L = digitalRead(sensorL);
      int R = !digitalRead(sensorR); 

      if(L==0 && R==0){
        cntL = 0;
        cntR = 0;
        break;
      }

    }

    while(true){
      line();

      int L = digitalRead(sensorL);
      int R = digitalRead(sensorR);

      if(L==1 && R==1) break;
    }
  }

  while(true){
    line();
  }
}