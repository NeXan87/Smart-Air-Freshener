#pragma once

#include "config.h"
#include <Bounce2.h>

void initStateMachine();
void updateStateMachine();
inline bool isLightOn();

extern State currentState;
extern Bounce button;
