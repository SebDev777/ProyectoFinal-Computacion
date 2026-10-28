#include <Arduino.h>
#include <Irisoled.h>

#include "setup.h"
#include "pir.h"
#include "display.h"
#include "task.h"

void setup()
{

  Serial.begin(115200);

  setupProject();
}

void loop()
{

  Task::update();

  if (newMovement())
  {

    Serial.println("New movement detected: Hello!");

    showFaceWithMessageFor(Irisoled::happy, "Hola!", 1500);
  }
}
