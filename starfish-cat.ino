// tiny urgent claws tug on your heartstrings and your nightmares

SYSTEM_THREAD(ENABLED);

#include "Adafruit_MLX90614.h"
#include "math.h"

// Pin use:
// thermal: D0, D1
// vibration: A1
// pneumatic: A0
// Servos: A4, A5, WKP, RX, TX, D2, D3

// legs
#define L0_PIN A4
#define L1_PIN A5
#define L2_PIN A7
#define L3_PIN RX
#define L4_PIN TX

/*
// ears
#define EL_PIN D2
#define ER_PIN D3
*/

// "eyes" (thermal)
#define I0_ADDR 0x40
#define I1_ADDR 0x45
#define I2_ADDR 0x50 // tail
#define I3_ADDR 0x55
#define I4_ADDR 0x5A

// sound
#define speakerPin A1

// pneumatic
#define pneumoPin A0

// tentacles

Adafruit_MLX90614 I0 = Adafruit_MLX90614(I0_ADDR);
Adafruit_MLX90614 I1 = Adafruit_MLX90614(I1_ADDR);
Adafruit_MLX90614 I2 = Adafruit_MLX90614(I2_ADDR);
Adafruit_MLX90614 I3 = Adafruit_MLX90614(I3_ADDR);
Adafruit_MLX90614 I4 = Adafruit_MLX90614(I4_ADDR);

// claws

Servo L0;
Servo L1;
Servo L2;
Servo L3;
Servo L4;


// servo purring

int purrPos=0;

// pneumatics
int pneumoCycles=1000;
int pneumoMinCycles=10;
int pneumoSpeedFactor=10;
int pneumoX=0;
int pneumoUpDown=1;

// temperature sensing
// should be able to calibrate cat by resetting
// every setup, he waits a few seconds and then curls all his claws in to indicate that he is taking temperature
// this will be considered "room temperature" and he will seek things that are at least warmThreshold degrees over room temperature
float skinThreshold=8;
float warmThreshold=6;
float liveThreshold=2;

float raw_input[5];
float input[5];
float calibrate[5];
int calAngle=20;

int lastOutput;
int outputDelay;
int outputDelayMin=6;
int outputDelayMax=15;

void setup() {

    Serial.begin(9600);

    Particle.function("knead",knead);
    Particle.function("double",doubleKnead);

    pinMode(L0_PIN,OUTPUT);
    pinMode(L1_PIN,OUTPUT);
    pinMode(L2_PIN,OUTPUT);
    pinMode(L3_PIN,OUTPUT);
    pinMode(L4_PIN,OUTPUT);

    pinMode(D7,OUTPUT);
    digitalWrite(D7,HIGH);

    pinMode(speakerPin,OUTPUT);
    digitalWrite(speakerPin,HIGH);

    pinMode(pneumoPin,OUTPUT);

    I0.begin();
    I1.begin();
    I2.begin();
    I3.begin();
    I4.begin();

    calibrateLegs();

    outputDelay=random(2*outputDelayMin,2*outputDelayMax+1)*500;

}

void loop() {
  getInput();

  parseInput();

}

void attachLegs() {
  L0.attach(L0_PIN);
  L1.attach(L1_PIN);
  L2.attach(L2_PIN);
  L3.attach(L3_PIN);
  L4.attach(L4_PIN);
}

void detachLegs() {
  L0.detach();
  L1.detach();
  L2.detach();
  L3.detach();
  L4.detach();
}

void calibrateLegs() {
  attachLegs();

  L0.write(0);
  L1.write(0);
  L2.write(0);
  L3.write(0);
  L4.write(0);

  delay(1000);

  L0.write(calAngle);
  L1.write(calAngle);
  L2.write(calAngle);
  L3.write(calAngle);
  L4.write(calAngle);

  delay(2500);

  // calibrate each leg and move it back when you're done
  // do it as an average

  for (int x=0; x<10; x++) {
    calibrate[0]=(calibrate[0]+I0.readObjectTempF())/2;
    delay(100);
  }
  L0.write(0);

  for (int x=0; x<10; x++) {
    calibrate[1]=(calibrate[1]+I1.readObjectTempF())/2;
    delay(100);
  }
  L1.write(0);

  for (int x=0; x<10; x++) {
    calibrate[2]=(calibrate[2]+I2.readObjectTempF())/2;
    delay(100);
  }
  L2.write(0);

  for (int x=0; x<10; x++) {
    calibrate[3]=(calibrate[3]+I3.readObjectTempF())/2;
    delay(100);
  }
  L3.write(0);

  for (int x=0; x<10; x++) {
    calibrate[4]=(calibrate[4]+I4.readObjectTempF())/2;
    delay(100);
  }
  L4.write(0);

  Serial.print(calibrate[0]);
  Serial.print("   ");
  Serial.print(calibrate[1]);
  Serial.print("   ");
  Serial.print(calibrate[2]);
  Serial.print("   ");
  Serial.print(calibrate[3]);
  Serial.print("   ");
  Serial.print(calibrate[4]);
  Serial.println();

  detachLegs();
}

int minInputPos() {  // gives back the minimum value of input array
  int min=0;
  for (int x=1; x<5; x++) {
    if (input[x]<input[min]) { min=x; }
  }
  return min;
}

int maxInputPos() {  // gives back the maximum value of input array
  int max=0;
  for (int x=1; x<5; x++) {
    if (input[x]>input[max]) { max=x; }
  }
  return max;
}

void getInput() {
  raw_input[0]=I0.readObjectTempF();
  raw_input[1]=I1.readObjectTempF();
  raw_input[2]=I2.readObjectTempF();
  raw_input[3]=I3.readObjectTempF();
  raw_input[4]=I4.readObjectTempF();

  for (int x=0; x<5; x++) {
    input[x]=raw_input[x]-calibrate[x];
  }

  for (int y=0; y<5; y++) {
    Serial.print(raw_input[y]); Serial.print(" > "); Serial.print(input[y]); Serial.print("  ");
  }
  Serial.println();

}

void parseInput() {

  int liveSum=0;
  int warmSum=0;
  int skinSum=0;

  for (int x=0; x<5; x++) {
    liveSum=liveSum+(input[x]>liveThreshold);
    warmSum=warmSum+(input[x]>warmThreshold);
    skinSum=skinSum+(input[x]>skinThreshold);
  }

  Serial.print(liveSum); Serial.print(", ");
  Serial.print(warmSum); Serial.print(", ");
  Serial.println(skinSum);

  if (liveSum>0) {
    if (warmSum>3 || skinSum>2) {
      Serial.println("Living warmth!");
      Serial.println(pneumoX);
        if (pneumoUpDown==1) {
        digitalWrite(pneumoPin,HIGH);
        delay(sqrt(pneumoSpeedFactor*pneumoX));
        digitalWrite(pneumoPin,LOW);
        delay(sqrt(pneumoSpeedFactor*pneumoX));
        pneumoX--;
        Serial.println("suckle up");
      }
      else {
        digitalWrite(pneumoPin,HIGH);
        delay(sqrt(pneumoSpeedFactor*pneumoX));
        digitalWrite(pneumoPin,LOW);
        delay(sqrt(pneumoSpeedFactor*pneumoX));
        pneumoX++;
        Serial.println("suckle down");
      }

      // suckle
      if (pneumoX>=pneumoCycles) {
        pneumoUpDown=1;
      }
      else if (pneumoX<=0) {
        pneumoUpDown=0;
      }
    }

    else {
      // knead the two warmest legs
      // wait to parse until outputDelay has passed
      if ((millis()-lastOutput)>outputDelay) {
        cryOn();
        lastOutput=millis();
        outputDelay=random(2*outputDelayMin,2*outputDelayMax+1)*500;
        int n = random(3,7);
        int maxInput1 = maxInputPos();
        input[maxInput1] = 0;
        int maxInput2 = maxInputPos();
        doubleKnead(String(n)+","+String(maxInput1)+","+String(maxInput2));
        delay(n*500);
        cryOff();
      }
    }

  }
}

int doubleKnead(String command) {  // knead two legs alternately
  int l1; // which legs
  int l2;

  Servo leg1;
  Servo leg2;

  Serial.println(command);

  int maxKneads = 0;
  char inputStr[64];
  command.toCharArray(inputStr,64);
  char *p = strtok(inputStr,",");
  maxKneads = atoi(p);
  p = strtok(NULL,",");
  l1 = atoi(p);
  p = strtok(NULL,",");
  l2 = atoi(p);

  int maxHeight = 45;  // number of cycles over which we space the kneading motion
  int delayShort = 200; // delay between individual claw motions
  int delayLong = 400;  // delay after you finish kneading
  float speedFactor = 5;  // changes the acceleration of the kneading. lower is faster.

  if (l1==0) {
    leg1.attach(L0_PIN);
  }
  if (l1==1) {
    leg1.attach(L1_PIN);
  }
  if (l1==2) {
    leg1.attach(L2_PIN);
  }
  if (l1==3) {
    leg1.attach(L3_PIN);
  }
  if (l1==4) {
    leg1.attach(L4_PIN);
  }

  if (l2==0) {
    leg2.attach(L0_PIN);
  }
  if (l2==1) {
    leg2.attach(L1_PIN);
  }
  if (l2==2) {
    leg2.attach(L2_PIN);
  }
  if (l2==3) {
    leg2.attach(L3_PIN);
  }
  if (l2==4) {
    leg2.attach(L4_PIN);
  }

  // lift leg 1

  for (int i=maxHeight; i>0; i--) {
      leg1.write(maxHeight-i);
      delay(sqrt(speedFactor*i));
  }

  for (int r=0; r<maxKneads; r++) {
    // leg 2 going up while leg 1 going down
    for (int i=maxHeight; i>0; i--) {
      leg1.write(i);
      leg2.write(maxHeight-i);
      delay(sqrt(speedFactor*i));
    }
    delay(delayShort);

    // now leg 2 going down while leg 1 going up
    for (int i=maxHeight; i>0; i--) {
      leg1.write(maxHeight-i);
      leg2.write(i);
      delay(sqrt(speedFactor*i));
    }
      delay(delayShort);
  }

  delay(delayShort);

  for (int i=maxHeight; i>0; i--) {
      leg1.write(i);
      delay(sqrt(speedFactor*i));
  }

  delay(delayLong);
  leg1.detach();
  leg2.detach();

  return maxKneads;

}

int knead(String command) {
  Servo servo;

  int m = 0;
  char inputStr[64];
  command.toCharArray(inputStr,64);
  char *p = strtok(inputStr,",");
  m = atoi(p);
  p = strtok(NULL,",");
  int l = atoi(p);

  int y = 20;
  int d = 200;
  int b = 400;
  int w = 75;

  if (l==0) {
    servo.attach(L0_PIN);
  }
  if (l==1) {
    servo.attach(L1_PIN);
  }
  if (l==2) {
    servo.attach(L2_PIN);
  }
  if (l==3) {
    servo.attach(L3_PIN);
  }
  if (l==4) {
    servo.attach(L4_PIN);
  }

  for (int r=0; r<m; r++) {
      for (int i=y; i>0; i--) {
          servo.write(y-i);
          delay(sqrt(w*i));
      }
      delay(d);
      for (int i=y; i>0; i--) {
          servo.write(i);
          delay(sqrt(w*i));
      }
      delay(d);
  }
  delay(b);
  servo.detach();

    return m;
}

// sound setoff

// meow

void cryOn() {
  digitalWrite(speakerPin,LOW);
}


void cryOff() {
  digitalWrite(speakerPin,HIGH);
}
