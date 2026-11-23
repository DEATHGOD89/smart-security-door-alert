/* ==============================================================================
 * PROJECT 3: Multi-Layer Smart Security Door Alert & Automated Entry System
 * DEVELOPED FOR: Robotics & STEM Educator Interview / Portfolio
 * AUTHOR: Bhupender Singh
 * 
 * CONCEPT (Kya hai yeh project?):
 * Yeh ek modern smart home security door system hai jo 2 levels par safety provide karta hai:
 * Level 1: Ultrasonic Sensor (HC-SR04) darwaze ke aage khade insaan ki distance measure karta hai.
 * Level 2: Magnetic Reed Switch detect karta hai ki darwaza physically Band (Closed) hai ya Khula (Open).
 * Agar koi visitor aage khada ho toh Welcome Chime bajti hai aur Servo motor lock kholti hai.
 * Lekin agar bina authorization ke darwaza forcibly khola jaye, toh Emergency Police Siren bajti hai!
 * ============================================================================== */

// --- LIBRARIES ---
#include <Servo.h>         // Micro servo motor (SG90) control karne ke liye library

// --- HARDWARE PIN CONFIGURATION ---
const int TRIG_PIN = 9;    // Ultrasonic sensor ka Trigger pin (Sound pulse bhejne ke liye)
const int ECHO_PIN = 10;   // Ultrasonic sensor ka Echo pin (Reflected pulse receive karne ke liye)
const int REED_PIN = 2;    // Magnetic Door Sensor (Reed switch) Digital Pin 2 pe connect hai
const int SERVO_PIN = 6;   // Servo motor ka PWM signal Pin 6 pe connect hai
const int BUZZER_PIN = 8;  // Alert buzzer Pin 8 pe connect hai
const int RED_LED = 4;     // Intruder / Danger indicator Red LED
const int GREEN_LED = 5;   // Normal / Welcome indicator Green LED

// --- CONSTANTS & THRESHOLDS ---
const int VISITOR_DISTANCE_CM = 40; // Darwaze ke 40 cm ke andar aane par visitor maana jayega
const int LOCK_ANGLE = 0;           // Servo ka locked position (0 degrees)
const int UNLOCK_ANGLE = 90;        // Servo ka unlocked position (90 degrees)

// --- OBJECT CREATION ---
Servo doorLock; // Servo motor ka object create kiya

void setup() {
  // Serial monitor start kiya taaki live status aur distance print kar sakein
  Serial.begin(9600);

  // Ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT); // Trigger pin se 40kHz sound waves fire ki jayengi
  pinMode(ECHO_PIN, INPUT);  // Echo pin se reflection receive karke pulse time naapa jayega

  // Door Reed Switch
  // INPUT_PULLUP: Jab magnet door ke paas hoti hai (Door closed), reed switch close ho jata hai -> LOW
  // Jab door khulta hai (Magnet door se door gayi), reed switch open ho jata hai -> HIGH
  pinMode(REED_PIN, INPUT_PULLUP);

  // Indicators & Outputs
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Servo motor ko pin 6 se attach kar rahe hain aur lock position par set kar rahe hain
  doorLock.attach(SERVO_PIN);
  doorLock.write(LOCK_ANGLE); // Default state: Door locked

  // System readiness indicator
  digitalWrite(GREEN_LED, HIGH);
  delay(500);
  digitalWrite(GREEN_LED, LOW);

  Serial.println(F("=================================================="));
  Serial.println(F("[SYSTEM]: SMART DOOR SECURITY SYSTEM ONLINE"));
  Serial.println(F("[STATUS]: Perimeter Secured & Armed"));
  Serial.println(F("=================================================="));
}

// --- HELPER FUNCTION: ULTRASONIC DISTANCE CALCULATION ---
// Yeh function distance in centimeters calculate karke return karta hai
long getDistanceInCm() {
  // Trigger pin ko 2 microseconds ke liye LOW karte hain taaki clean HIGH pulse generate ho sake
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // 10 microsecond ka HIGH pulse bhejte hain (Sensor 8 cycle sonic burst fire karega)
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // pulseIn() function Echo pin par wave aane tak ka round-trip time (microseconds) naapta hai
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout (max ~5 meters)

  if (duration == 0) {
    return 999; // Agar koi obstacle na mile ya timeout ho jaye
  }

  // Formula: Distance = (Duration * Speed of Sound) / 2
  // Speed of sound = 343 m/s = 0.0343 cm/microsecond
  // Round trip distance / 2 = Duration * 0.0343 / 2 = Duration / 58.2
  long distanceCm = duration / 58.2;
  return distanceCm;
}

void loop() {
  // Step 1: Distance measure karte hain
  long distance = getDistanceInCm();

  // Step 2: Door status read karte hain (LOW = Closed, HIGH = Open)
  int isDoorOpen = digitalRead(REED_PIN);

  // Status print karte hain debugging ke liye
  Serial.print(F("Distance: "));
  Serial.print(distance);
  Serial.print(F(" cm | Door: "));
  if (isDoorOpen == HIGH) {
    Serial.println(F("OPEN"));
  } else {
    Serial.println(F("CLOSED"));
  }

  // --- SCENARIO A: INTRUSION / FORCED ENTRY (Danger Alert!) ---
  // Agar door khula hai aur koi authorized visitor detection nahi hua:
  if (isDoorOpen == HIGH && distance > VISITOR_DISTANCE_CM) {
    Serial.println(F(">>> [SECURITY BREACH]: Door Forced Open! Intruder Alert! <<<"));
    
    // Red LED flashing aur Police siren effect
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    // Siren tone 1
    tone(BUZZER_PIN, 1000); // 1000 Hz sound frequency
    delay(150);
    // Siren tone 2
    tone(BUZZER_PIN, 500);  // 500 Hz sound frequency
    delay(150);
    digitalWrite(RED_LED, LOW);
    delay(50);
  }

  // --- SCENARIO B: NORMAL VISITOR WELCOME ---
  // Agar door closed hai aur visitor 40cm ke paas aa gaya:
  else if (distance <= VISITOR_DISTANCE_CM && distance > 0) {
    Serial.println(F("[EVENT]: Visitor Approaching Door. Unlocking..."));

    noTone(BUZZER_PIN); // Siren band rakhein
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH); // Green LED ON

    // Sweet Welcome Chime (Ding-Dong)
    tone(BUZZER_PIN, 800); delay(120);
    tone(BUZZER_PIN, 1200); delay(180);
    noTone(BUZZER_PIN);

    // Servo motor ghumakar lock unlock karte hain (90 degrees)
    doorLock.write(UNLOCK_ANGLE);
    delay(3000); // 3 seconds tak darwaza khula rakhein

    // Wapas darwaza lock kar dete hain
    doorLock.write(LOCK_ANGLE);
    digitalWrite(GREEN_LED, LOW);
  }

  // --- SCENARIO C: IDLE NORMAL STATE ---
  else {
    // Sab kuch normal hai, sound band rakhein
    noTone(BUZZER_PIN);
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    doorLock.write(LOCK_ANGLE); // Ensure locked
    delay(100);
  }
}
