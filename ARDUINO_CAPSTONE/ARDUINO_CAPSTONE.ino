#include<Servo.h>

// off on off on off off
#define level1Steps 0
#define level2Steps 2300
#define level3Steps 4600

#define enPin 2
#define pulPin 3
#define dirPin 4
#define servo1Pin 5
#define servo2Pin 6
#define ir1Pin 7
#define ir2Pin 8
#define ir3Pin 9
#define ir4Pin 10
#define ir5Pin 11
#define ir6Pin 12
#define lock1 14 
#define lock2 15
#define lock3 16
#define lock4 17
#define lock5 18
#define lock6 19
#define jamPin 13

Servo servo1;
Servo servo2;

bool execute = 0;

// ========== AUTO-LOCK VARIABLES ==========
unsigned long unlockTime[7] = {0, 0, 0, 0, 0, 0, 0};  // When each drawer was unlocked
bool autoLockEnabled[7] = {false, false, false, false, false, false, false};  // Which drawers should auto-lock
const unsigned long AUTO_LOCK_DELAY = 5000; 

void setup() 
{
  Serial.begin(9600);
  
  // Stepper motor pins `1`
  pinMode(enPin, OUTPUT);
  pinMode(pulPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  digitalWrite(enPin, LOW);
  
  // Servos
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo1.write(90);
  servo2.write(90);
  
  // IR sensors
  pinMode(ir1Pin, INPUT);
  pinMode(ir2Pin, INPUT); 
  pinMode(ir3Pin, INPUT);
  pinMode(ir4Pin, INPUT);
  pinMode(ir5Pin, INPUT);
  pinMode(ir6Pin, INPUT);
  
  // Locks
  pinMode(lock1, OUTPUT);
  pinMode(lock2, OUTPUT);
  pinMode(lock3, OUTPUT);
  pinMode(lock4, OUTPUT);
  pinMode(lock5, OUTPUT);
  pinMode(lock6, OUTPUT);

  pinMode(jamPin, INPUT);
  
  Serial.println("BRBQueue Ready");
}

void loop() 
{
  // ========== CHECK FOR AUTO-LOCK ==========
  unsigned long currentTime = millis();
  for (int i = 1; i <= 6; i++) {
    if (autoLockEnabled[i] && (currentTime - unlockTime[i] >= AUTO_LOCK_DELAY)) {
      // Time's up! Lock this drawer
      digitalWrite(13 + i, LOW);  // lock1 is pin 14, so lock1 = 13+1
      autoLockEnabled[i] = false;
      Serial.print("AUTO-LOCK: Drawer ");
      Serial.print(i);
      Serial.println(" locked after 5 seconds");
    }
  }
  
  if (Serial.available())
  {  
    String command = Serial.readStringUntil("\n");
    command.trim();
    
    Serial.print("Received: ");
    Serial.println(command);
    
    // SORTING COMMANDS
    if (command.equals("sortDrawer1") || command.equals("sortDrawer4") || 
        command.equals("sortDrawer2") || command.equals("sortDrawer5") || 
        command.equals("sortDrawer3") || command.equals("sortDrawer6")) {
      execute = 1;
      irCheck(command);  // ADDED: Check IR before sorting
    }
    
    if (execute == 1)
      leveling(command);
    execute = 0;
    
    // UNLOCK COMMANDS
    if (command.equals("UNLOCK 1")) {
      digitalWrite(lock1, HIGH);
      unlockTime[1] = millis();
      autoLockEnabled[1] = true;
      Serial.println("OK: Drawer 1 UNLOCKED (will auto-lock in 5s)");
    }
    else if (command.equals("UNLOCK 2")) {
      digitalWrite(lock2, HIGH);
      unlockTime[2] = millis();
      autoLockEnabled[2] = true;
      Serial.println("OK: Drawer 2 UNLOCKED (will auto-lock in 5s)");
    }
    else if (command.equals("UNLOCK 3")) {
      digitalWrite(lock3, HIGH);
      unlockTime[3] = millis();
      autoLockEnabled[3] = true;
      Serial.println("OK: Drawer 3 UNLOCKED (will auto-lock in 5s)");
    }
    else if (command.equals("UNLOCK 4")) {
      digitalWrite(lock4, HIGH);
      unlockTime[4] = millis();
      autoLockEnabled[4] = true;
      Serial.println("OK: Drawer 4 UNLOCKED (will auto-lock in 5s)");
    }
    else if (command.equals("UNLOCK 5")) {
      digitalWrite(lock5, HIGH);
      unlockTime[5] = millis();
      autoLockEnabled[5] = true;
      Serial.println("OK: Drawer 5 UNLOCKED (will auto-lock in 5s)");
    }
    else if (command.equals("UNLOCK 6")) {
      digitalWrite(lock6, HIGH);
      unlockTime[6] = millis();
      autoLockEnabled[6] = true;
      Serial.println("OK: Drawer 6 UNLOCKED (will auto-lock in 5s)");
    }
    
    else if (command.equals("LOCK 1")) {
      digitalWrite(lock1, LOW);
      autoLockEnabled[1] = false;
      Serial.println("OK: Drawer 1 LOCKED");
    }
    else if (command.equals("LOCK 2")) {
      digitalWrite(lock2, LOW);
      autoLockEnabled[2] = false;
      Serial.println("OK: Drawer 2 LOCKED");
    }
    else if (command.equals("LOCK 3")) {
      digitalWrite(lock3, LOW);
      autoLockEnabled[3] = false;
      Serial.println("OK: Drawer 3 LOCKED");
    }
    else if (command.equals("LOCK 4")) {
      digitalWrite(lock4, LOW);
      autoLockEnabled[4] = false;
      Serial.println("OK: Drawer 4 LOCKED");
    }
    else if (command.equals("LOCK 5")) {
      digitalWrite(lock5, LOW);
      autoLockEnabled[5] = false;
      Serial.println("OK: Drawer 5 LOCKED");
    }
    else if (command.equals("LOCK 6")) {
      digitalWrite(lock6, LOW);
      autoLockEnabled[6] = false;
      Serial.println("OK: Drawer 6 LOCKED");
    }
    
    // LOCK ALL
    else if (command.equals("LOCK_ALL")) {
      for (int x = 14; x <= 19; x++) {
        digitalWrite(x, LOW);
        autoLockEnabled[x - 13] = false;  // Cancel all auto-locks
      }
      Serial.println("OK: All drawers LOCKED");
    }

    else if (command.equals("CHECK_IR")) {
      Serial.print("IR_STATUS:");
      Serial.print(digitalRead(ir1Pin) == 0 ? "FULL" : "EMPTY");
      Serial.print(",");
      Serial.print(digitalRead(ir2Pin) == 0 ? "FULL" : "EMPTY");
      Serial.print(",");
      Serial.print(digitalRead(ir3Pin) == 0 ? "FULL" : "EMPTY");
      Serial.print(",");
      Serial.print(digitalRead(ir4Pin) == 0 ? "FULL" : "EMPTY");
      Serial.print(",");
      Serial.print(digitalRead(ir5Pin) == 0 ? "FULL" : "EMPTY");
      Serial.print(",");
      Serial.print(digitalRead(ir6Pin) == 0 ? "FULL" : "EMPTY");
      Serial.println();
    }
    
    // Unknown command
    else {
      Serial.print("UNKNOWN: ");
      Serial.println(command);
    }
  }
}

void irCheck(String command)
{
  if (command.equals("sortDrawer1") && digitalRead(ir1Pin) == 0)
  { Serial.println("Drawer 1 Full"); Serial.println("BLOCKED:1"); execute = 0; }
  if (command.equals("sortDrawer2") && digitalRead(ir2Pin) == 0)
  { Serial.println("Drawer 2 Full"); Serial.println("BLOCKED:2"); execute = 0; }
  if (command.equals("sortDrawer3") && digitalRead(ir3Pin) == 0)
  { Serial.println("Drawer 3 Full"); Serial.println("BLOCKED:3"); execute = 0; }
  if (command.equals("sortDrawer4") && digitalRead(ir4Pin) == 0)
  { Serial.println("Drawer 4 Full"); Serial.println("BLOCKED:4"); execute = 0; }
  if (command.equals("sortDrawer5") && digitalRead(ir5Pin) == 0)
  { Serial.println("Drawer 5 Full"); Serial.println("BLOCKED:5"); execute = 0; }
  if (command.equals("sortDrawer6") && digitalRead(ir6Pin) == 0)
  { Serial.println("Drawer 6 Full"); Serial.println("BLOCKED:6"); execute = 0; }
}

void leveling(String command)
{
  Serial.println("Executing...");
  if (command.equals("sortDrawer1"))
  { levelDirection(1); levelLoop(level1Steps); tilting(1); if (jamCheck() == 0){levelDirection(0); levelLoop(level1Steps); }else Serial.println("JAM_DETECTED");}
  else if (command.equals("sortDrawer2"))
  { levelDirection(1); levelLoop(level2Steps); tilting(1); if (jamCheck() == 0){levelDirection(0); levelLoop(level2Steps); }else Serial.println("JAM_DETECTED");}
  else if (command.equals("sortDrawer3"))
  { levelDirection(1); levelLoop(level3Steps); tilting(1); if (jamCheck() == 0){levelDirection(0); levelLoop(level3Steps); }else Serial.println("JAM_DETECTED");}
  else if (command.equals("sortDrawer4"))
  { levelDirection(1); levelLoop(level1Steps); tilting(0); if (jamCheck() == 0){levelDirection(0); levelLoop(level1Steps); }else Serial.println("JAM_DETECTED");}
  else if (command.equals("sortDrawer5"))
  { levelDirection(1); levelLoop(level2Steps); tilting(0); if (jamCheck() == 0){levelDirection(0); levelLoop(level2Steps); }else Serial.println("JAM_DETECTED");}
  else if (command.equals("sortDrawer6"))
  { levelDirection(1); levelLoop(level3Steps); tilting(0); if (jamCheck() == 0){levelDirection(0); levelLoop(level3Steps); }else Serial.println("JAM_DETECTED");}
  Serial.println("Done!");
}

void levelDirection(bool down)
{
  if (down == 1) Serial.print("Going to "); 
  else Serial.print("Going back from ");
  digitalWrite(dirPin, down);
}

void levelLoop(int revolutions)
{
  if (revolutions == level1Steps) Serial.println("Level 1...");
  else if (revolutions == level2Steps) Serial.println("Level 2...");
  else if (revolutions == level3Steps) Serial.println("Level 3...");

  for (int s = 0; s <= revolutions; s++)
  {
    digitalWrite(pulPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(pulPin, LOW);
    delayMicroseconds(500);
  }
}

void tilting(bool left)
{ 
  byte tiltDir1 = 55;
  byte tiltDir2 = 125;
  delay(1000);

  Serial.print("Tilting to ");

  if (left == 1)
  {
    Serial.println("Left...");
    tiltDir1 = 55;
    tiltDir2 = 125;
  }
  else
  {
    Serial.println("Right...");
    tiltDir1 = 125;
    tiltDir2 = 55;
  }

  servo1.write(tiltDir1);
  servo2.write(tiltDir2);
  delay(2000);

  Serial.println("Tilting back...");

  servo1.write(90);
  servo2.write(90);
  delay(1000);
}

bool jamCheck(){
  if (digitalRead(jamPin) == 0){
    return 1;
  }
  else
    return 0;
}