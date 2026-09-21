# Task architecture

So we need the following tasks:

- logging task
- mixer task

But now we need the following tasks:

- Ultrasonic sensor task which polls at around 20Hz (should be adjustable).
- Ultrasonic sensor task should push to a queue which has a time stamp and the on period, and also we should have a time out too, and perhaps we should add a timeout flag too (the reason why we use time stamp is because we can add additional filtering afterwards. For now, we don't need to filter it). Also, we want the queue it self to leak when it overflows.
- if we wanted to do some filtering, filtering is weird because a low pass filter is a first order filter that needs to keep track of $a[n-1]$, meanwhile, a second order filter such as the Butterworth's filter needs $a[n -1]$ and also $a[n - 2]$
- Servo task should just wait until it gets notified, and then it takes the servo angle mutex, takes the servo angle and writes it to ledc's PWM.
- We then have the `BluetoothRC` task, whose job is to update the state of all of the buttons to see whether it is pressed or not, and push the state of buttons, into a queue
- We will have a mode selector task which is a simple finite state machine that detect edges on the select button and the start button, and if we detect an edge, on either one of them, we perhaps stop the `AutomaticModeTask` task and starts the `ManualModeTask` task (we can perhaps stop it directly, or make the mode wait. If we stop it directly, we might actually save on memory, and we can also guarantee mutual exclusion (because you don't want them to run at the same time), although, we would run into things like not unlocking a mutex, or being interrupted mid-write to something, etc, and also, the responsibility of cleaning up such as setting resetting the speed to 0 and stuff would fall onto the `ModeSelectTask`) Also, we would have to think about how RC commands are going to propogate, because, if we have a single struct for all UP, DOWN, LEFT, RIGHT, TRIANGLE, CIRCLE, SQUARE, etc... Then the `ModeSelector` task would have consumed it already, so either `ModeSelector` just peeks at it, which would be bad if the state is idle because the queue wouldn't have a consumer.

## Justification for not having the speed, speedLimit and servo angle be a queue

So, the reason why the `speed` and `speedLimit` isn't a queue is because, it I think it represents the set point of what the robot is doing, which should be stored even if we don't get anything from the queue. 

Similarly, the servo angle represents the angle at which the servo is at, which is fine because, the servo itself already has the PID loop to take care of the angle, so all we need to store is what angle it currently is at.

Meanwhile, an ultrasonic reading represents a time series, and therefore, we kinda need the timestamp at which we initialized the pulse so that we can use it to perhaps try to do some filtering, like low-pass filtering (actually doesn't require timestamp).

So as for manual mode and automatic mode, manual mode is actually quite simple, because, it's very similar to what we've always done:

- we wait for notification
- perhaps we can read the `modeState`, which is either `IDLE`, `MANUAL` or `AUTO`, and we continue if we are in the desired state
- we then read the `bluetothRCState` or something, and then we do some stuff accordingly like moving the robot forward, stopping the robot, setting the servo to a certain position, etc. The one who is notifying `manualMode` is going to be the mode selector, because the mode selector will be sort of the orchestrator, and essentially, we will notify the `manualMode` task every time the `bluetoothRC` task notifies us, or adds something to the queue or something
- So I guess `modeSelector` will just run the `manualMode` and `automaticMode` function itself, or notifies another task. If it's just notification based, then it might be possible to just turn it into a function instead, where we first read whether we're in `MANUAL` or `AUTO`, and then we notify, or run the short task accordingly.

- the task (or function) for automatic mode and manual mode will be as follows:
  - we read the necessary state, like, if we're in manual mode, and we get a LEFT HIGH, then we turn left, if we get like a CIRCLE RISING EDGE, we update the servo angle, etc.
  - if we're in automatic mode, then we actually consume from the ultrasonic queue (or perhaps we make that a state instead, since the consumer shouldn't be blocking and constantly active). But the difficult thing about the automatic mode is that it's very difficult to make it notification based, but perhaps it's fine because we should decide what to once we have some ultrasonic sensor, but again, ultrasonic sensor polling should be asynchronous to bluetooth polling rate, so perhaps we actually need to make it task based. Perhaps we do something like, before we wait to take from the ultrasonic queue, or before we wait to get notified by the ultrasonic task, we check whether the mode is AUTO, if not, then... it would be nice if we could wait until the mode is back to AUTO, like a blocking wait, but if not, then if the ultrasonic polling task is constantly notifying the AUTO task, then it should be able to react when it's back to AUTO mode...


  So although it's asymmetric, here's a decently clean architecture:

  - we have an RC bluetooth task polls periodically and then update the button state, and then notifies the `modeSelector` or `orchistrator` or something task. We can call it like the `BluetoothRCOrchistrator` or something like that.
  - the orchistrator task will be responsible for checking whether there's an edge on the SELECT button, or the START button, and then updating a global `bluetoothState` struct or something.
  - then if it's like, in MANUAL mode, then it either runs the manual mode function, or notifies the manual mode task
  - if it's in AUTO mode, then perhaps if the AUTO mode is running asynchornously, it would have already detected that we've switch it to AUTO mode, and then it would do its thing, like, for an obstacle avoidance thing, it would probably have states like, SEARCH which just rotates the robot, or MOVE which just moves the robot forward, and the state machine updates every time we get a new ultrasonic reading. But when we switch to manual mode, I guess we can just force the state machine to not run even if we get new data that tries to run it, and when we see that we're back in AUTO mode, I guess it can resume. I think this is fine because, at the boundary, when we switch from AUTO to MANUAL, even if the robot was moving, manual mode would have stopped the robot already, and in the boundary case when we switch from MANUAL to AUTO, even if the ROBOT is idle, if we were in SEARCHING or something, and we've seen that the distance is big, then we switch to MOVE mode, but the thing with AUTO mode is that we are going to need to cause side-effects everytime the state machine runs because, let's say that you were in SEARCHING state, and then we switch to manual mode, and then we go back to AUTO mode and we're still in the searching state, however, our robot isn't actually rotating or anything, so that's an inconvience, but it shouldn't be too bad.

So essentially, for a normal state machine that's running continuously, it's actually fine to cause side effects on entry, because, we know that we're the only one causing side effects, like, if enter a state and toggle the LED on, we can assume that while we're in that state, LED is on, and no one else will go and toggle the LED off. But if we have something that toggled the LED off, then I guess we'll need to decide whether we should toggle the LED on forcefully or we should just let it be. Actually, in this case, if we can acutally poll whether we have the LED on, we wouldn't have to naively toggle the LED on every time, like, we can check whether speed.linear = 1, speed.angular = 0 or something and if it isn't, then we can set the speed and notify the mixer task.

## Nice refactoring

So it would be nice if we refactor the code as follows:

- for every shared struct, we should also store the mutex that gets initialized inside of it (that is, assuming that the mutex is initialized only once)


# TODO


