#include <SoftwareSerial.h>
//the identifier of this XBee
#define THIS_GOAL 1
//max number of goals that can be tracked
#define MAX_HITS 20
//seconds allowed for shot before it defaults
#define TIME_TO_HIT 5
//set pin macros
#define SENSOR 7
#define IR_LED 6
#define LED 8
#define BUTTON 2
#define aInput 26 //for cutoff voltage
byte hitArray[MAX_HITS] = {0}; //this stores hit and miss data
bool running = false; //true when this goal is on
unsigned long previousMillis = 0;
byte currHit = 0; //tracks the position in the drill
bool drillStarted = false; //tracks whether goal is on
byte numberOfGoals = 10; //DRILL PARAMETER : Number of goals for this drill. Acquired through SoftwareSerial from ESP8266
unsigned long interval = 5000; //DRILL PARAMETER : Interval between acquired through SoftwareSerial from ESP8266
byte nextGoal = 0;
float voltage;
SoftwareSerial Serial3(9, 10); // RX | TX;
void setup(){
  //set pins and stuff
  pinMode(SENSOR, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(IR_LED, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(aInput, INPUT);
  Serial.begin(9600);
  if(THIS_GOAL == 1){
    Serial3.begin(9600);
  }
  unsigned long y=0;  //Use this to generate a more random seed
  for (int k=0; k<32; k++) y += (y << 3) ^ analogRead(A0); //Any unconnected analog pin
  randomSeed(y);
}
void loop(){
  //loop through reading the network until one of my start() is started or
  // my address is called by turnOnGoal()
  // waitForHit() and when hit or over time, check if drill is over
  //if not pickNextGoal() with a randomized XBee
  voltage = readVolts();
  if(THIS_GOAL == 1 && digitalRead(BUTTON)==HIGH){
    numberOfGoals = 10;
    startGoals();
  }
  if(THIS_GOAL == 1 && !running && Serial3.available() && voltage >= 10.5){
    if(Serial3.read() == '%'){ // % defines start byte
      /*
      int numG = Serial3.read();
      Serial.println(numG);
      numberOfGoals = (byte) numG;
      Serial.println(numberOfGoals);
      int timer = (int) Serial3.read();
      Serial.println(timer);
      interval = (unsigned long) timer;
      interval = interval * 1000;
      Serial.println(interval);
      if(Serial3.read() == '#'){ //if successful line ending
        Serial3.print('#');
        drillStarted = true;
        startGoals();
      }
      */
      Serial3.print('#'); // # defines end byte
      drillStarted = true;
      startGoals();
    }
  }
  if(running && voltage >= 10.5){
    waitForHit();
  }
  else{
    //check for my spot
    readFrame();
  }
}
void startGoals(){
  //set the current shot to 1 and turn on a goal
  currHit = 1;
  turnOnGoal();
}
void pickNextGoal(){
  //pick next goal of 3
  digitalWrite(LED, LOW);
  noTone(IR_LED); // turn off emitter
  running = false;
  delay(100);
  if(currHit < numberOfGoals){
    currHit++;
    nextGoal = randomizer();
    if(nextGoal == THIS_GOAL){
      delay(900);
      turnOnGoal();
    }
    else{
      delay(400);
      sendData(nextGoal, 0xFF, currHit);
    }
  }
  else{
    sendData(0xFF, 0x06, currHit);//tell other goals we done
    finishDrill();
  }
}
int randomizer(){
  //as long as not THIS_GOAL
  int randNum = (int) random(1, 3);
  return randNum;
}
void turnOnGoal(){
  //Turns on this goal
  running = true;
  previousMillis = millis();
  digitalWrite(LED, HIGH);
  tone(IR_LED, 38000);
}
void waitForHit(){
  //Runs when goal is on and waiting for sensor data
  //loop for 5 seconds with light turned on
  //log a hit when hit or log miss after 5 minutes
  unsigned long currentMillis = millis();
  if(digitalRead(SENSOR)==HIGH){
    delay(5);
    if(digitalRead(SENSOR)==HIGH){
      hitArray[currHit] = (byte) 1;
      pickNextGoal();
    }
  }
  else if(currentMillis - previousMillis >= interval){
    hitArray[currHit] = (byte) 2;
    pickNextGoal();
  }
}
void finishDrill(){
  drillStarted = false;
  if(THIS_GOAL != 1){ //if not receiving data, send it
    unsigned int lagTime = (THIS_GOAL - 1) * 1000; //initialize a lag time for aggregating data
    delay(lagTime);
    aggregateData();
  }
  else{
    bool arrayFull = false;
    //Serial.println("receiving aggregate");
    while(!arrayFull){
      receiveAggregateData();
      //Serial.println("scanning array");
      arrayFull = true;
      for(int x=1; x<=numberOfGoals; x++){
        //Serial.print(hitArray[x]);
        if (hitArray[x] == 0){
          arrayFull = false;
        }
      }
    }
    
    Serial3.print('%'); //start char
    for(int x=1; x<=numberOfGoals; x++){
      Serial3.print(hitArray[x]);
    }
    Serial3.print('#'); //end char
  }
  while(true){
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }
}
/////////////////////////////
////XBEE HELPER FUNCTIONS////
/////////////////////////////
void readFrame(){ //For mesh network
  byte program = 0;
  byte hitCount = 0;
  if(Serial.available()>17){
    while(Serial.peek() != 0x7E){ //clear anything before the start if it isn't the start byte
    Serial.read();
   }
  if(Serial.available() > 17){//wait for rest of bytes to come in
     for(int x=0;x<15;x++){
       Serial.read(); //discard bytes preceeding command
     }
     program = Serial.read(); //this is the first command byte
     hitCount = Serial.read(); //second data byte
     Serial.read();
   }
   if(program == 0xFF){
     turnOnGoal();
     currHit = hitCount;
   }
   else if(program == 0x06){
    finishDrill();
   }
 }
}
void sendData(int node, byte program, byte hitCount){
 //For mesh network
 //Sends program byte to a specific node where FF is broadcast
 byte chex = 0; //For mesh network
 Serial.write(0x7E); //start delimitter
 Serial.write(0x00);
 Serial.write(0x10);
 Serial.write(0x10);
 Serial.write(0x00); //do not request a response
 Serial.write(0x00);
 if(node == 0xFF){
   Serial.write(0x00);
   Serial.write(0x00);
   Serial.write(0x00);
   Serial.write(0x00);
   Serial.write(0x00);
   Serial.write(0xFF);
   Serial.write(0xFF);
   Serial.write(0xFF);
   Serial.write(0xFE);
   Serial.write(0x00);
   Serial.write(0x00);
   Serial.write(program);
   Serial.write(hitCount);
   chex = 0xF4 - program - hitCount;
 }
 else{
   Serial.write(0x13);
   Serial.write(0xA2);
   Serial.write(0x00);
   Serial.write(0x40);
   if(node == 1){
    Serial.write(0xC8);
    Serial.write(0xEB);
    Serial.write(0x89);
    Serial.write(0xFF);
    Serial.write(0xFE);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(program);
    Serial.write(hitCount);
    chex = 0xC1 - program - hitCount;
   }
   else if(node == 2){
    Serial.write(0xC8);
    Serial.write(0xEB);
    Serial.write(0x89);
    Serial.write(0xFF);
    Serial.write(0xFE);
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(program);
    Serial.write(hitCount);
    chex = 0xC4 - program - hitCount;
   }
 }
 Serial.write(chex);
 Serial.flush();
}
void aggregateData(){ //sends hit/miss data to base goal
  byte chex = 0xC1; //what checksum is without data
  byte frameLength = (byte) 14 + numberOfGoals;
  Serial.write(0x7E);
  Serial.write(0x00);
  Serial.write(frameLength);
  Serial.write(0x10);
  Serial.write(0x00); //do not request a response
  //64-bit address of target, Node1
  Serial.write(0x00);
  Serial.write(0x13);
  Serial.write(0xA2);
  Serial.write(0x00);
  Serial.write(0x40);
  Serial.write(0xC8);
  Serial.write(0xEB);
  Serial.write(0x89);
  Serial.write(0xFF);
  Serial.write(0xFE);
  Serial.write(0x00);
  Serial.write(0x00);
  //DATA
  for(int i=1; i<=numberOfGoals; i++){
    Serial.write(hitArray[i]);
    chex -= hitArray[i];
  }
  Serial.write(chex);
  Serial.flush();
}
void receiveAggregateData(){ //base goal receives hit/miss data
  if(Serial.available()>17){
    while(Serial.peek() != 0x7E){ //clear anything before the start if it isn't the start byte
      Serial.read();
   }
  if(Serial.available() >= 16 + numberOfGoals){//wait for rest of bytes to come in
    for(int x=0;x<15;x++){
      Serial.read(); //discard bytes preceeding
    }
    for(int x=1; x<=numberOfGoals; x++){
      byte curr = Serial.read();
      if(curr != 0){
        hitArray[x] = curr; //Add incoming values to existing Array
      }
    }
    Serial.read(); //discard the checksum
   }
 }
}

float readVolts(){ //computes voltage of battery for cutoff voltage, +/- 10% accuracy
  unsigned long long int VoltAdds = 0;
  float voltage;
  for(int i=0; i<1000; i++){ //take many samples to ensure accuracy of voltage read
    int currV = analogRead(aInput); //analogRead is not linearly related to Voltage on Teensy
    VoltAdds += currV;
  }
  VoltAdds/=1000;
  voltage = 0.01226*VoltAdds; //corresponds to voltage
  return voltage; //cutoff should be 10.046 actual at LOWEST
}
