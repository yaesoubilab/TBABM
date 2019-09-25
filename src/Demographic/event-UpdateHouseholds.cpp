#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using namespace StatisticalDistributions;

EventFunc TBABM::UpdateHouseholds(void)
{
  EventFunc ef = 
    [this] (double t, SchedulerT scheduler) {
      for (auto it = households.begin(); it != households.end(); it++) {
        if (!it->second)
          continue;

        data.householdsCount.Record(t, +1);
      }
      Schedule(t + 365, UpdateHouseholds());
      return true;
    };

  return ef;
}
