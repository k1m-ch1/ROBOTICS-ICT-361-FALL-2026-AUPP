# Plan

Sign mapping:

- V sign (index + middle fingers up): Move Forward
- Fist Move: BACKWARD,70
- Index finger pointing left: left turn
- Index finger pointing right: right turn
- Open palm: Stop

The plan is this, because we want the sign recognition to be mutually exclusive, instead of training some classification model which is like the proper way to do things, we will just use some heuristics to determine which sign we're intending to do. We'll just use a bunch of conditional, with of course, a built in priority of what it should recognize.

So mediapipe will essentially give me the points of each "landmark" of our finger, such as base as the finger, or tip of the finger, etc.

To determine which fingers are up, I have some ideas:

- the naive idea is to set some heuristic like "if euclidean distance between tip to base is greater than x pixels, then we consider it to be up"
- of course, this is prone to scaling issues, because like, it's hard to do projection, so another idea is to do scaling based on the maximum euclidean distance, however, this isn't robust since the hand-closed gesture and the hand-open gesture is going to be treated the same
- so if we want a reference scale, perhaps it would be best to use another point on the hand that is going to be:
  - visible regardless of whether the hand is open or closed

Well, if we know a good heuristic to determine whether hands are open and closed, then we have the following if-else conditional:

- Open palm: we just check whether all 5 fingers are up
- Fist move: just check whether all 5 fingers are down (perhaps it can be independent of the thumb)
- V sign: check whether index and middle finger are both up
- Only Index finger pointing up: check for two cases:
  - take the tip of the finger and the base of the finger and compute the vector from the base of the finger to the tip of the finger
  - use it to find the angle using the dot product formula
  - if the angle is in this particular range, treat it as a index pointing to the left
  - if the angle is in some other particular range, treat it as an index pointing to the right
- Otherwise, do nothing

Apparantly, here are some good references:

- wrist → middle-finger MCP
- index MCP → pinky MCP

We'll need to measure it empirically and set it as parameters.

If we don't want to scale it, another robust heuristic is to make sure that if we draw a vector from the finger tip to the finger MCP, we want to specify 2 vectors, one from the MCP to the tip, another from the MCP to the pip, and then we consider the hand to be open when the magnitude of the MCP to the finger tip is greater than that from the MCP to the pip. Similarly, angle can be determined by specifying the vector between the MCP and the tip, given that then are considered "open".

Let's try the euclidean distance between the tip and wrist instead
