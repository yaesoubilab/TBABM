#include "../../include/TBABM/TBABM.h"
#include <Empirical.h>
#include <Exponential.h>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace StatisticalDistributions;
using std::vector;
using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

EventFunc TBABM::MortalityCheck(weak_p<Individual> idv_w)
{
  EventFunc ef = 
    [this, idv_w](double t, SchedulerT scheduler) {
      // printf("[%d] MortalityCheck: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));

      double samplingWidth = 1/12.;

      // Make sure the individual is alive and HIV-positive
      auto idv = idv_w.lock();
      if (!idv)
        return true;
      if (idv->dead || idv->hivStatus != HIVStatus::Positive)
        return true;

      // Retrieve constant parameters m, p, and m_30
      double m = params["HIV_m"].Sample(rng);
      double p = params["HIV_p"].Sample(rng);
      double m_30 = params["HIV_m_30"].Sample(rng);

      // Calculate current CD4 count
      double CD4 = idv->CD4count(t, m_30);

      // Calculate rate of death for this individual, varying
      // on CD4 count
      double M_c = m*exp(-1*p*CD4);

      // Calculate time to HIV-related death, and schedule it if
      // it occurs before next MortalityCheck and CD4 count is low
      // Unit: YEARS
      double timeToMortality = Exponential(M_c)(rng.mt_);

      if (timeToMortality < samplingWidth)
        Schedule(t + 365.*timeToMortality, Death(idv_w, DeathCause::HIV));
      else
        Schedule(t + 365.*samplingWidth, MortalityCheck(idv_w));

      return true;
    };

  return ef;
}
