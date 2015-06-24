#pragma once

#include "Scheduler.h"

inline Scheduler& SystemScheduler()
{
    static Scheduler s(16);
    return s;
}
