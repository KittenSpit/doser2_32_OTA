#pragma once
#include <vector>
#include "Model.h"

bool loadSchedule();
void saveSchedule();
std::vector<ScheduleItem>& scheduleRef();
void schedulerLoop();
