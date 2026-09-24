#include "logging.h"

LogMessage logMessage;

void setup(){
  loggingInit();
}

void loop(){
  logMessage.timestamp = millis();
  logMessage.logSource = RC;
  sprintf(logMessage.text, "hello");
  xQueueSend(logQueueHandle, &logMessage, 0);
  delay(1000);
}
