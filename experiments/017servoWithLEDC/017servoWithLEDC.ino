#define SERVO_PIN 17

#define FREQ 50
#define RES 16

#define DUTY_MIN 0.025f
#define DUTY_MAX 0.125f

char buffer[256];

void setup(){
  Serial.begin(115200);
  ledcAttach(SERVO_PIN, FREQ, RES);
}

void loop(){
  for (int i = 0; i < 100; i++){
  float t = i/100.0f;
  float duty = (1 - t)*DUTY_MIN + t*DUTY_MAX;
  uint32_t dutyToWrite = (uint32_t) (duty * (1 << RES));
  sprintf(buffer, "Setting duty cycle to  %f which maps to %lu\r\n", duty, dutyToWrite);
  ledcWrite(SERVO_PIN, dutyToWrite);
  Serial.print(buffer);
  delay(200);
  }
}
