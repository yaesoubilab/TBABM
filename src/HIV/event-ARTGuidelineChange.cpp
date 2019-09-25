#include "../../include/TBABM/TBABM.h"
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace StatisticalDistributions;
using std::vector;
using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

EventFunc TBABM::ARTGuidelineChange(void)
{
  EventFunc ef = 
    [this](double t, SchedulerT scheduler) {
      // printf("[%d] ARTGuidelineChange\n", (int)t);
      for (auto it = seekingART.begin(); it != seekingART.end();) {
        auto idv = (*it).lock();
        if (!idv || idv->dead) {
          it++; continue;
        }

        bool initiateART;
        double m_30 = params["HIV_m_30"].Sample(rng);
        double CD4 = idv->CD4count(t, m_30);

        // [0,1] is cast to bool here
        if (idv->tb.GetTBStatus(t) == TBStatus::Infectious)
          initiateART = fileData["HIV_p_art_tb"].getValue(0, 0, CD4, rng);
        else
          initiateART = fileData["HIV_p_art"].getValue(0, 0, CD4, rng);

        if (ARTEligible(t, idv) && initiateART) {
          Schedule(t, ARTInitiate(idv));
          it = seekingART.erase(it);
        }
        else it++;
      }

      Schedule(t + 365, ARTGuidelineChange());

      return true;
    };

  return ef;
}
