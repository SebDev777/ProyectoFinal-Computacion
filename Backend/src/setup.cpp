#include <Arduino.h>

#include "setup.h"
#include "display.h"
#include "pir.h"
#include "notes.h"
#include "task.h"

#define SPEAKER_PIN 19
#define SPEAKER_CHANNEL 0
#define RESOLUTION 8

const int melody[] = {
    NOTE_B4,
    NOTE_CS5,
    NOTE_GS4,
    NOTE_A4,
    NOTE_FS4,
    NOTE_DS5,
    NOTE_CS5,
    0};

const int noteDurations[] = {
    119,
    119,
    119,
    119,
    119,
    119,
    119,
    119};

void startMelody()
{

    for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++)
    {

        int frequency = melody[i];
        int duration = noteDurations[i];

        if (frequency > 0)
        {
            ledcWriteTone(SPEAKER_CHANNEL, frequency);
        }
        else
        {
            ledcWriteTone(SPEAKER_CHANNEL, 0);
        }

        // This code runs in a FreeRTOS task; yield the processor while the
        // tone plays instead of blocking with Arduino's busy wait.
        vTaskDelay(pdMS_TO_TICKS(duration));

        ledcWriteTone(SPEAKER_CHANNEL, 0);
    }
}

void setupProject()
{
    setupPIR();
    setupDisplay();

    ledcSetup(SPEAKER_CHANNEL, 2000, RESOLUTION);
    ledcAttachPin(SPEAKER_PIN, SPEAKER_CHANNEL);

    if (!Task::spawn(startMelody))
    {
        Serial.println("Could not start melody task");
    }
}
