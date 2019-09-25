#include "../../include/TBABM/TBABM.h"
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace StatisticalDistributions;
using std::vector;
using EventFunc  = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

EventFunc TBABM::ARTInitiate(weak_p<Individual> idv_w)
{
  EventFunc ef = 
    [this, idv_w](double t, SchedulerT scheduler) {
      auto idv = idv_w.lock();
      if (!idv)
        return true;
      if (idv->dead || idv->hivStatus != HIVStatus::Positive)
        return true;

      double m_30   = params["HIV_m_30"].Sample(rng);
      int CD4       = idv->CD4count(t, m_30);
      // printf("[%d] ARTInitiate: %ld::%lu, CD4=%d\n", (int)t, idv->householdID, \
      // std::hash<Pointer<Individual>>()(idv), CD4);


      idv->ARTInitTime = t;
      idv->ART_init_CD4 = CD4;
      idv->onART = true;

      data.hivPositiveART.Record(t, +1);

      return true;
    };

  return ef;
}
