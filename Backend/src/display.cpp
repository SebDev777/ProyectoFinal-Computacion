#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Irisoled.h>

#include "display.h"
#include "task.h"

namespace
{
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr int8_t OLED_RESET = -1;
constexpr uint8_t OLED_SDA_PIN = 21;
constexpr uint8_t OLED_SCL_PIN = 22;
constexpr uint8_t OLED_ADDRESSES[] = {0x3C, 0x3D};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned char *currentFace = Irisoled::normal;
uint32_t faceRevision = 0;
bool displayReady = false;
bool animationRunning = false;
bool blinkScheduled = false;
bool lookScheduled = false;
bool lastLookWasLeft = false;

void drawFace(const unsigned char *face);
void drawMessage(const char *message);
uint8_t findDisplayAddress();

void scheduleNextBlink();
void scheduleNextLook();
void startBlink();
void finishBlink();
void startLook();
void finishLook();

bool idleIsAvailable()
{
    return currentFace == Irisoled::normal && !animationRunning;
}

void scheduleNextBlink()
{
    if (currentFace != Irisoled::normal || blinkScheduled)
    {
        return;
    }

    blinkScheduled = Task::delay(random(8000, 15000), startBlink);
}

void scheduleNextLook()
{
    if (currentFace != Irisoled::normal || lookScheduled)
    {
        return;
    }

    lookScheduled = Task::delay(random(6000, 12000), startLook);
}

void startBlink()
{
    blinkScheduled = false;

    if (!idleIsAvailable())
    {
        scheduleNextBlink();
        return;
    }

    animationRunning = true;
    drawFace(Irisoled::blink);

    if (!Task::delay(45, finishBlink))
    {
        finishBlink();
    }
}

void finishBlink()
{
    animationRunning = false;
    drawFace(currentFace);

    if (currentFace == Irisoled::normal)
    {
        scheduleNextBlink();
        scheduleNextLook();
    }
}

void startLook()
{
    lookScheduled = false;

    if (!idleIsAvailable())
    {
        scheduleNextLook();
        return;
    }

    animationRunning = true;

    // Every horizontal look goes to the other side next time.
    lastLookWasLeft = !lastLookWasLeft;
    drawFace(lastLookWasLeft ? Irisoled::look_left : Irisoled::look_right);

    if (!Task::delay(random(450, 850), finishLook))
    {
        finishLook();
    }
}

void finishLook()
{
    animationRunning = false;
    drawFace(currentFace);

    if (currentFace == Irisoled::normal)
    {
        scheduleNextBlink();
        scheduleNextLook();
    }
}

void drawFace(const unsigned char *face)
{
    if (!displayReady || face == nullptr)
    {
        return;
    }

    display.clearDisplay();
    display.drawBitmap(0, 0, face, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.display();
}

void drawMessage(const char *message)
{
    if (!displayReady || message == nullptr)
    {
        return;
    }

    display.fillRect(0, 52, SCREEN_WIDTH, 12, SSD1306_BLACK);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(47, 54);
    display.print(message);
    display.display();
}

uint8_t findDisplayAddress()
{
    for (const uint8_t address : OLED_ADDRESSES)
    {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0)
        {
            return address;
        }
    }

    return 0;
}
} // namespace

void setupDisplay()
{
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

    const uint8_t address = findDisplayAddress();
    if (address == 0)
    {
        Serial.println("OLED not found. Check 3V3, GND, SDA (21), and SCL (22).");
        return;
    }

    if (!display.begin(SSD1306_SWITCHCAPVCC, address))
    {
        Serial.println("OLED initialization failed.");
        return;
    }

    displayReady = true;
    randomSeed(micros());
    drawFace(currentFace);
    scheduleNextBlink();
    scheduleNextLook();
    Serial.printf("OLED initialized at I2C address 0x%02X\n", address);
}

void showFace(const unsigned char *face)
{
    if (face == nullptr)
    {
        return;
    }

    ++faceRevision;
    currentFace = face;
    animationRunning = false;
    drawFace(currentFace);

    if (currentFace == Irisoled::normal)
    {
        scheduleNextBlink();
        scheduleNextLook();
    }
}

void showFaceFor(const unsigned char *face, unsigned long duration)
{
    showFace(face);

    const uint32_t revision = faceRevision;
    Task::delay(duration, [revision]()
                {
                    if (revision == faceRevision)
                    {
                        showFace(Irisoled::normal);
                    }
                });
}

void showFaceWithMessageFor(
    const unsigned char *face,
    const char *message,
    unsigned long duration)
{
    showFace(face);
    drawMessage(message);

    const uint32_t revision = faceRevision;
    Task::delay(duration, [revision]()
                {
                    if (revision == faceRevision)
                    {
                        showFace(Irisoled::normal);
                    }
                });
}
