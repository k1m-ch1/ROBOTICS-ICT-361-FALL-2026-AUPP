#include "logging.h"
#include "frameReader.h"
#include "frameDecoder.h"
#include "mixer.h"
#include "motors.h"
#include "servo.h"

void setup(){
  loggingInit();
  motorsInit();
  mixerInit();
  servoInit();
  servoWrite(140.0f);

  frameDecoderInit();
  frameReaderInit();
}

void loop(){
  //LogMessage logMessage;
  //logMessage.logSource = RC;
  //logMessage.timestamp = millis();
  //sprintf(logMessage.text, "hello");
  //xQueueSend(logQueueHandle, &logMessage, 0);
  //delay(1000);
}
