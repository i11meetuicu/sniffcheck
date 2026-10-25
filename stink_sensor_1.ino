// SniffCheck - Stink Sensor (version 3)
// Arduino UNO R4 WiFi + Grove Base Shield
// Grove Air Quality Sensor on A0, Grove Buzzer on D8.
//
// New in v3:
//  - longer warm-up (60 s) so the sensor settles before calibrating
//  - self-correcting baseline: if the air gets cleaner than the saved
//    "clean air" value (e.g. leftover sanitizer fades away), the baseline
//    slowly follows it down, so it can't stay stuck too high
//  - recalibrate any time without unplugging: send "c" over serial
//    (the SniffCheck app's "Recalibrate" button does this)

const int SENSOR_PIN = A0;
const int BUZZER_PIN = 8;
const int WARMUP_SECONDS = 60;
const int CALIBRATE_SAMPLES = 50;

float baseline = 0;

// average 10 quick readings so the number is less jumpy
int readSensor() {
  long total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(SENSOR_PIN);
    delay(10);
  }
  return total / 10;
}

void beep(int ms) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(ms);
  digitalWrite(BUZZER_PIN, LOW);
}

void calibrate() {
  Serial.println("Learning what clean air smells like...");
  long total = 0;
  for (int i = 0; i < CALIBRATE_SAMPLES; i++) {
    total += readSensor();
    delay(100);
  }
  baseline = (float)total / CALIBRATE_SAMPLES;
  if (baseline < 1) baseline = 1;
  Serial.print("Clean air baseline = ");
  Serial.println(baseline);
  Serial.println("Ready! Hold the sensor near a sock.");
  beep(100); delay(100); beep(100);  // two beeps = "ready"
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial && millis() < 5000) { }

  beep(100);  // short beep = "I'm on!"

  Serial.println("Stink Sensor starting...");
  Serial.println("Warming up - keep the sensor in fresh air!");
  for (int s = WARMUP_SECONDS; s > 0; s--) {
    Serial.print("Warming up: ");
    Serial.print(s);
    Serial.println(" seconds left");
    delay(1000);
  }
  calibrate();
}

void loop() {
  // "c" from the app (or Serial Monitor) = recalibrate in clean air now
  while (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == 'c' || ch == 'C') calibrate();
  }

  int raw = readSensor();

  // self-correct: if the air is cleaner than our baseline, follow it down
  if (raw < baseline) {
    baseline = baseline * 0.9 + raw * 0.1;
    if (baseline < 1) baseline = 1;
  }

  float ratio = raw / baseline;  // 1.0 = same as clean air

  // ratio 1.0 -> score 0, ratio 1.8 -> score 100 (tuned for the cube case)
  int score = (int)((ratio - 1.0) / 0.8 * 100.0);
  if (score < 0) score = 0;
  if (score > 100) score = 100;

  String verdict;
  if (score < 20)      verdict = "Fresh";
  else if (score < 50) verdict = "Hmm...";
  else if (score < 80) verdict = "Stinky";
  else                 verdict = "BIOHAZARD";

  // The web app reads this line
  Serial.print("raw=");
  Serial.print(raw);
  Serial.print(" base=");
  Serial.print((int)baseline);
  Serial.print(" score=");
  Serial.print(score);
  Serial.print(" verdict=");
  Serial.println(verdict);

  // Alerts for people who can't smell
  if (score >= 80) {
    beep(300);   // long beep = biohazard
  } else if (score >= 50) {
    beep(60);    // short beep = stinky
  }

  delay(500);
}
