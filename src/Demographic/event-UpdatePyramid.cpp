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

EventFunc TBABM::UpdatePyramid(void)
{
  int interval = 365;

  EventFunc ef = 
    [this, interval] (double t, SchedulerT scheduler) {
      for (auto it = population.begin(); it != population.end(); it++) {
        if (!*it || (*it)->dead)
          continue;

        auto idv = *it;

        int age = idv->age(t);
        int sex = idv->sex == Sex::Male ? 0 : 1;

        data.pyramid.UpdateByAge(t-1, sex, age, +1);

        if (age >= 15 && idv->age(t-interval) < 15) {
          data.populationChildren.Record(t, -1);
          data.populationAdults.Record(t, +1);
        }
      }

      Schedule(t + interval, UpdatePyramid());
      return true;
    };

  return ef;
}
