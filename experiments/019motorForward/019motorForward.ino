#include "motors.h"
#include "mixer.h"
#include "logging.h"

void setup(){
  loggingInit();
  motorsInit();
  mixerInit();

  LogMessage logMessage;
  logMessage.timestamp = millis();
  logMessage.logSource = MOTOR;
  sprintf(logMessage.text, "hello world\r\n");
  xQueueSend(logQueueHandle, &logMessage, portMAX_DELAY);

  xSemaphoreTake(speedMutex, portMAX_DELAY);
  speed.linear = 0.0f;
  speed.angular = -0.25f;
  xSemaphoreGive(speedMutex);
  xTaskNotifyGive(mixerTaskHandle);

  delay(5000);

  xSemaphoreTake(speedMutex, portMAX_DELAY);
  speed.linear = 0.0f;
  speed.angular = 0.0f;
  xSemaphoreGive(speedMutex);
  xTaskNotifyGive(mixerTaskHandle);
}

void loop(){

}
