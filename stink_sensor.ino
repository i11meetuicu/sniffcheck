// SniffCheck - Stink Sensor (version 2, with buzzer)
// Arduino UNO R4 WiFi + Grove Base Shield
// Grove Air Quality Sensor on A0, Grove Buzzer on D8.
// Learns what clean air smells like at startup, then prints a
// Stink Score from 0 (fresh) to 100 (biohazard) over USB serial.

const int SENSOR_PIN = A0;
const int BUZZER_PIN = 8;
const int WARMUP_SECONDS = 30;
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

void loop() {
  int raw = readSensor();
  float ratio = raw / baseline;  // 1.0 = same as clean air

  // ratio 1.0 -> score 0, ratio 2.5 -> score 100
  int score = (int)((ratio - 1.0) / 1.5 * 100.0);
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
