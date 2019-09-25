#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace StatisticalDistributions;
using std::vector;
using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

// Algorithm S11: Leave current household to form new household
EventFunc TBABM::LeaveHousehold(weak_p<Individual> idv_w)
{
  EventFunc ef = 
    [this, idv_w](double t, SchedulerT scheduler) {
      // printf("[%d] LeaveHousehold: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));
      auto idv = idv_w.lock();
      if (!idv)
        return true;
      if (idv->dead)
        return true;

      Schedule(t, CreateHousehold(idv_w, \
            weak_p<Individual>{}, \
            {},\
            {}));

      return true;
    };

  return ef;
}
