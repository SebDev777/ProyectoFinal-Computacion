#include <Arduino.h>
#include "task.h"

namespace
{
constexpr uint8_t MAX_TASKS = 10;

struct ScheduledTask
{
    bool active;
    bool repeating;

    unsigned long executeAt;
    unsigned long interval;

    TaskCallback callback;
};

ScheduledTask tasks[MAX_TASKS] = {};
}

bool Task::delay(
    unsigned long milliseconds,
    TaskCallback callback)
{
    if (!callback)
    {
        return false;
    }

    for (int i = 0; i < MAX_TASKS; i++)
    {

        if (!tasks[i].active)
        {

            tasks[i].active = true;
            tasks[i].repeating = false;

            tasks[i].executeAt = millis() + milliseconds;
            tasks[i].interval = 0;

            tasks[i].callback = callback;

            return true;
        }
    }

    return false;
}

bool Task::every(
    unsigned long interval,
    TaskCallback callback)
{
    if (!callback || interval == 0)
    {
        return false;
    }

    for (int i = 0; i < MAX_TASKS; i++)
    {

        if (!tasks[i].active)
        {

            tasks[i].active = true;
            tasks[i].repeating = true;

            tasks[i].executeAt = millis() + interval;
            tasks[i].interval = interval;

            tasks[i].callback = callback;

            return true;
        }
    }

    return false;
}

void Task::update()
{

    unsigned long currentTime = millis();

    for (int i = 0; i < MAX_TASKS; i++)
    {

        if (!tasks[i].active)
        {
            continue;
        }

        if ((long)(currentTime - tasks[i].executeAt) >= 0)
        {

            if (tasks[i].repeating)
            {
                tasks[i].executeAt =
                    currentTime + tasks[i].interval;
            }
            else
            {
                // Free this slot before user code runs so its callback can
                // schedule another task even when the queue is otherwise full.
                tasks[i].active = false;
            }

            TaskCallback callback = tasks[i].callback;
            if (!tasks[i].repeating)
            {
                tasks[i].callback = nullptr;
            }

            callback();
        }
    }
}

bool Task::spawn(TaskCallback callback)
{
    if (!callback)
    {
        return false;
    }


    TaskCallback *taskCallback =
        new TaskCallback(callback);

    const BaseType_t result = xTaskCreate(
        [](void *parameter)
        {
            TaskCallback *callback =
                static_cast<TaskCallback *>(parameter);

            (*callback)();

            delete callback;

            vTaskDelete(NULL);
        },
        "Task",
        4096,
        taskCallback,
        1,
        NULL);

    if (result != pdPASS)
    {
        delete taskCallback;
        return false;
    }

    return true;
}
