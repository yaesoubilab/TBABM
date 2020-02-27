#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <Bernoulli.h>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace StatisticalDistributions;
using std::vector;
using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

EventFunc TBABM::HIVInfectionCheck(weak_p<Individual> idv_w)
{
  EventFunc ef = 
    [this, idv_w](double t, SchedulerT scheduler) {
      // printf("[%s %d] HIVInfectionCheck\n", idv->Name().c_str(), (int)t);
      // printf("Use count: %ld\n", idv.use_count());
      auto idv = idv_w.lock();
      if (!idv)
        return true;
      if (idv->dead || idv->hivStatus == HIVStatus::Positive)
        return true;

      int startYear   = constants["startYear"];
      int agWidth     = constants["ageGroupWidth"];
      int currentYear = startYear + (int)t/365;
      int gender      = idv->sex == Sex::Male ? 0 : 1;
      int age         = idv->age(t); // in years
      auto spouse     = idv->spouse.lock();

      // Probability of getting infected in the next 5yr according to Thembisa
      long double p_getInfected_thembisa {0};

      // Probability we will use, after attenuating the Thembisa risk
      long double p_getInfected          {0};

      // Whether or not the individual will get infectged
      bool getsInfected               {false};
      
      // If they get infected, this is when it will happen, expressed as a
      // delta from the current time.
      // Unit: years
      long double timeToProspectiveInfection {Uniform(0., agWidth)(rng.mt_)};

      // Different risk profiles for HIV+ and HIV- spouse. As of 12/03/18, these
      // profiles are the same, and are drawn from Excel Thembisa 4.1
      if (spouse && !spouse->dead && spouse->hivStatus == HIVStatus::Positive)
        p_getInfected_thembisa = \
          fileData["HIV_risk_spouse"].getValue(currentYear, gender, age, rng);
      else
        p_getInfected_thembisa = \
          fileData["HIV_risk"].getValue(currentYear, gender, age, rng);

      // "HIV_risk_attenuation" is a value between [0,1]
      p_getInfected = \
        params["HIV_risk_attenuation"].Sample(rng) * \
        p_getInfected_thembisa;

      getsInfected = Bernoulli(p_getInfected)(rng.mt_);

      if (getsInfected)
        Schedule(t + 365*timeToProspectiveInfection, HIVInfection(idv_w));
      else
        Schedule(t + 365, HIVInfectionCheck(idv_w));

      return true;
    };

  return ef;
}
