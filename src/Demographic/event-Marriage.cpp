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

// Algorithm S13: Marriage
EventFunc TBABM::Marriage(weak_p<Individual> m_weak, weak_p<Individual> f_weak)
{
  EventFunc ef = 
    [this, m_weak, f_weak](double t, SchedulerT scheduler) {

      // printf("[%d] Marriage, populationSize=%lu, people: m=%ld::%lu f=%ld::%lu\n", (int)t, population.size(), m->householdID, std::hash<Pointer<Individual>>()(m), f->householdID, std::hash<Pointer<Individual>>()(f));
      auto m = m_weak.lock();
      auto f = f_weak.lock();
      assert(m != f);
      if (!m || !f)
        return true;
      if (m->dead || f->dead)
        return true;
      if (m->spouse.lock() || f->spouse.lock())
        return true;

      bool canDivorce {true};

      if (households[m->householdID]->size() == 1) {
        // The female will join the male's household
        ChangeHousehold(f, t, m->householdID, HouseholdPosition::Spouse);

        if (f->offspring.size() > 0)
          canDivorce = false;

        for (auto idv : f->offspring)
          ChangeHousehold(idv, t, m->householdID, HouseholdPosition::Offspring);

      } else if (households[f->householdID]->size() == 1) {
        // Male joins female household
        ChangeHousehold(m, t, f->householdID, HouseholdPosition::Spouse);

        if (m->offspring.size() > 0)
          canDivorce = false;

        for (auto idv : f->offspring)
          ChangeHousehold(idv, t, f->householdID, HouseholdPosition::Offspring);

      } else {
        // Couple forms new household?
        if (params["coupleFormsNewHousehold"].Sample(rng) == 1) {
          auto hid = nHouseholds++;
          bool can_trace = params["TB_CT_frac_visit"].Sample(rng);
          households[hid] = std::make_shared<Household>(t, hid, can_trace);

          ChangeHousehold(m, t, hid, HouseholdPosition::Head);
          ChangeHousehold(f, t, hid, HouseholdPosition::Spouse);

          if (f->offspring.size() > 0 || m->offspring.size() > 0)
            canDivorce = false;

          for (auto idv : f->offspring)
            ChangeHousehold(idv, t, hid, HouseholdPosition::Offspring);
          for (auto idv : m->offspring)
            ChangeHousehold(idv, t, hid, HouseholdPosition::Offspring);

        } else {
          ChangeHousehold(f, t, m->householdID, HouseholdPosition::Spouse);

          if (f->offspring.size() > 0)
            canDivorce = false;

          for (auto idv : f->offspring)
            ChangeHousehold(idv, t, m->householdID, HouseholdPosition::Offspring);
        }
      }

      m->spouse = f;
      f->spouse = m;

      m->marriageStatus = MarriageStatus::Married;
      f->marriageStatus = MarriageStatus::Married;

      m->marriageDate = t;
      f->marriageDate = t;

      // Will they divorce?
      if (params["probabilityOfDivorce"].Sample(rng) == 1 && canDivorce) {
        // Time to divorce
        double coupleAvgAge = (m->age(t) + f->age(t))/(2*365);
        double yearsToDivorce = fileData["timeInMarriage"].getValue(0,0,coupleAvgAge,rng);
        int daysToDivorce = 365 * yearsToDivorce;
        Schedule(t + daysToDivorce, Divorce(m, f));
      }


      // Time to first birth
      double yearsToFirstBirth = fileData["timeToFirstBirth"].getValue(0,0,f->age(t),rng);
      int daysToFirstBirth = 365 * yearsToFirstBirth;

      Schedule(t + daysToFirstBirth - 9*30, Pregnancy(m, f));

      data.marriages.Record(t, +1);

      return true;
    };

  return ef;
}
