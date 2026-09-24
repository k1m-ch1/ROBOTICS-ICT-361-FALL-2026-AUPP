#pragma once

float asymNormalizedMap(float raw, float minLeft, float maxLeft, float minRight,
                        float maxRight);

float mapFloat(float x, float x0, float x1, float y0, float y1);

typedef enum { EDGE_FALLING, EDGE_RISING, EDGE_CHANGE } EdgeType;

// although there's detectEdge in rc.cpp, since this is c++, I hope function
// overloading works

bool detectEdge(bool prevState, bool currentState, EdgeType edgeType);
