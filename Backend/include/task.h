#ifndef TASK_H
#define TASK_H

#include <functional>

using TaskCallback = std::function<void()>;

class Task
{
public:
    // Returns false when the scheduler has no free slot or the callback is invalid.
    static bool delay(unsigned long milliseconds, TaskCallback callback);
    static bool every(unsigned long interval, TaskCallback callback);
    static bool spawn(TaskCallback callback);

    static void update();
};

#endif
