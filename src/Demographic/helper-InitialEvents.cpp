#include "../../include/TBABM/TBABM.h"
#include <Exponential.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using namespace StatisticalDistributions;

// Unit of dt is years
void TBABM::InitialEvents(weak_p<Individual> idv_w, double t, double dt)
{
  auto idv = idv_w.lock();
  if (!idv)
    return;

  int gender = idv->sex == Sex::Male ? 0 : 1;
  double age = idv->age(t);
  auto startYear = constants["startYear"];

  // Unit of both of these is years
  double timeToDeath   = fileData["naturalDeath"].getValue(startYear+(int)t/365, gender, age, rng);
  double timeToLookingScale = params["timeToLookingScale"].Sample(rng);
  double timeToLooking = timeToLookingScale * fileData["timeToLooking"].getValue(0, gender, age, rng);

  if (timeToDeath < dt)
    Schedule(t + 365*timeToDeath, Death(idv, DeathCause::Natural));

  if ((idv->marriageStatus == MarriageStatus::Single ||
        idv->marriageStatus == MarriageStatus::Divorced) &&
      timeToLooking < dt)
    Schedule(t + 365*timeToLooking, SingleToLooking(idv));

  if (params["TB_p_initial_treatment"].Sample(rng)) {
    data.tbTxExperiencedAdults.Record(t, +1);

    data.tbTreatmentEnd.Record(t, +1);
    data.tbCompletedTreatment.Record(t, +1);
    data.tbRecoveries.Record(t, +1);
    data.tbLatent.Record(t, +1);

    idv->tb.SetTreatmentComplete();
  }

  return;
}
