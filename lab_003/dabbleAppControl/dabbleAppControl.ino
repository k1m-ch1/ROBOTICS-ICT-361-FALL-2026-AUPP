#include "logging.h"
#include "dabbleOrchestrator.h"
#include "dabbleManualMode.h"
#include "dabbleRC.h"
#include "ultrasonic.h"
#include "servo.h"
#include "mixer.h"
#include "motors.h"

void setup(){
  // these tasks are like, independent tasks
  loggingInit();
  motorsInit();
  servoInit();
  //ultrasonicInit();
  mixerInit();

  // start up order: 
  //1. `dabbleManualMode`: because it just starts up and halts until someone notifies it, meaning that it will create a task handle, and then wait until `dabbleOrchestrator` notifies it which would have guaranteed that `dabbleOrchestrator` initialized its modes variables and stuff
  //2. `dabbleOrchestrator`: we need this now because we need to initialize the modes that the auto-mode needs
  //4. `dabbleRCTask`: can now call `dabbleOrchestrator` by its handle
  //3. `dabbleAutoMode`: `dabbleOrchestrator` has already initialized the modes mutex so it is fine


  // TODO: initialize the mixer, take the speedLimit mutex and set it as something you can configure in here
  dabbleManualModeInit();
  dabbleOrchestratorInit();
  dabbleRCInit();
  // not going to initialize the auto mode for now
}

void loop(){
}
