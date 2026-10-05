#include "logging.h"
#include "frameReader.h"
#include "frameDecoder.h"

void setup(){
  loggingInit();
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
