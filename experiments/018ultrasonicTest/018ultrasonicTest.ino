#include "logging.h"
#include "ultrasonic.h"

UltrasonicData currentUltrasonicData = {};
LogMessage ultrasonicLogMessage = {};

void setup(){
  loggingInit();
  ultrasonicInit();
}

void loop(){
  // constantly take from the ultrasonic queue in a blocking way
  xQueueReceive(ultrasonicQueueHandle, &currentUltrasonicData, portMAX_DELAY);
  ultrasonicLogMessage.logSource = ULTRASONIC;
  ultrasonicLogMessage.timestamp = millis();

  float distanceCentimeters = getDistanceCentimeters(currentUltrasonicData.pulsePeriod);
  sprintf(ultrasonicLogMessage.text, "distance: %f", distanceCentimeters);
  xQueueSend(logQueueHandle, &ultrasonicLogMessage, 0);
}
