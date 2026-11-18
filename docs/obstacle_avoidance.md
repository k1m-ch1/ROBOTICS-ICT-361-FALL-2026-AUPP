# Plan

For obstacle avoidance, inference using YOLO takes quite a bit of time

So the idea is that we have one loop that reads from the camera and does line following (processing is fast if we just use HSV segmentation)

Meanwhile, if we need data like "is there a box in the frame", we essentially start another frame that just processes frames that the main loop gives to it. The main loop puts the latest frames into the queue, and only puts it into the queue when it sees that not only was the frame consumed, the thread has like increased the semaphore or something like that and use that to synchronize.

When we see a box, we verify with another heuristic like "is most of the color in the bounding box red" etc...

Now, after we've done all of that, and we do see a red box, we essentially pass it into a feedforward routing like "rotate left 1s, forward 2s, rotate right 1s, forward 2s,...."
