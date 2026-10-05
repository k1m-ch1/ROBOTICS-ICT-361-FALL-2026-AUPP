#define RX1 16
#define TX1 17

void setup(){
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RX1, TX1);
}

void loop(){
  while(Serial.available() > 0){
    Serial1.printf("%X ", Serial.read());
  }
}
