#include <Arduino.h>
#include "pir.h"

#define PIR_PIN 2

bool previousState = LOW;

void setupPIR()
{
    pinMode(PIR_PIN, INPUT);
}

bool movementDetected()
{
    return digitalRead(PIR_PIN) == HIGH;
}

bool newMovement()
{
    bool currentState = movementDetected();

    bool newMovementDetected =
        (currentState == HIGH && previousState == LOW);

    previousState = currentState;

    return newMovementDetected;
}