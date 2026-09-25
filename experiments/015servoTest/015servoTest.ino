#include "servo.h" 
#include "config.h"
#include "logging.h"
#include <stdint.h>

void setup(){
  loggingInit();
  servoInit();
}

void loop(){
  for (int i = 0; i < 181; i++){
  servoWrite(i);
  delay(200);
  // 180*500 = 4000
  }
}
