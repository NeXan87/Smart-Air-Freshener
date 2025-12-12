#pragma once

#include "config.h"
#include "utils.h"
#include <Bounce2.h>

void initStateMachine();
void updateStateMachine(SprayMode currentMode);
inline bool isLightOn();

extern State currentState;
extern Bounce button;
