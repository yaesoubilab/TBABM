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

// Algorithm S12: Change of marital status from single to looking
EventFunc TBABM::SingleToLooking(weak_p<Individual> idv_w)
{
  EventFunc ef = 
    [this, idv_w](double t, SchedulerT scheduler) {
      // printf("[%d] SingleToLooking: %s, %ld::%lu\n", (int)t, idv->sex == Sex::Male ? "Male" : "Female", idv->householdID, std::hash<Pointer<Individual>>()(idv));
      auto idv = idv_w.lock();
      if (!idv)
        return true;

      if (idv->dead || \
          idv->marriageStatus == MarriageStatus::Married)
        return true;

      idv->marriageStatus = MarriageStatus::Looking;

      if (idv->sex == Sex::Male)
        maleSeeking.push_back(idv_w);
      else
        femaleSeeking.push_back(idv_w);

      data.singleToLooking.Record(t, +1);
      return true;
    };

  return ef;
}
