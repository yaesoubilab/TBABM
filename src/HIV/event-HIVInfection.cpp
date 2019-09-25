#include "../../include/TBABM/TBABM.h"
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace StatisticalDistributions;
using std::vector;
using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

void HIVInfectionLogger(shared_p<Individual> idv, double t)
{
  std::cout << termcolor::on_magenta << 
    "[" << 
    std::left << std::setw(12) << 
    idv->Name() << 
    std::setw(5) << std::right << 
    (int)t << "] HIV infection" << 
    termcolor::reset << std::endl;
}

EventFunc TBABM::HIVInfection(weak_p<Individual> idv_w)
{
  EventFunc ef = 
    [this, idv_w](double t, SchedulerT scheduler) {

      // Have to be alive and seronegative to get infected
      auto idv = idv_w.lock();
      if (!idv)
        return true;
      if (idv->dead || idv->hivStatus == HIVStatus::Positive)
        return true;

      // HIVInfectionLogger(idv, t);

      idv->hivStatus = HIVStatus::Positive;

      // Decide CD4 count and value of 'k', and record as undiagnosed
      idv->initialCD4 = params["CD4"].Sample(rng);
      idv->kgamma = params["kGamma"].Sample(rng);
      idv->t_HIV_infection = t;
      idv->hivDiagnosed = false;

      // Immediately schedule possible VCT diagnosis, and begin checking
      // their HIV-related mortality
      Schedule(t, VCTDiagnosis(idv_w));
      Schedule(t, MortalityCheck(idv_w));

      // Reevaluate risk for tuberculosis, and change risk window
      idv->tb.RiskReeval(t);

      int sex {idv->sex == Sex::Male ? 0 : 1};
      int age {idv->age(t)};

      data.hivPositive.Record(t, +1);
      data.hivNegative.Record(t, -1);
      data.hivInfections.Record(t, +1);
      data.hivPositivePyramid.UpdateByAge(t, sex, age, +1);
      data.hivInfectionsPyramid.UpdateByAge(t, sex, age, +1);

      return true;
    };

  return ef;
}
