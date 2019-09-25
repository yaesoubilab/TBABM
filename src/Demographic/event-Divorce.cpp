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

// Algorithm S14: Divorce
EventFunc TBABM::Divorce(weak_p<Individual> m_weak, weak_p<Individual> f_weak)
{
  EventFunc ef = 
    [this, m_weak, f_weak](double t, SchedulerT scheduler) {

      // printf("[%d] Divorce, populationSize=%lu, people: m=%ld::%lu f=%ld::%lu\n", (int)t, population.size(), m->householdID, std::hash<Pointer<Individual>>()(m), f->householdID, std::hash<Pointer<Individual>>()(f));

      // If someone is dead they can't divorce
      auto m = m_weak.lock();
      auto f = f_weak.lock();
      if (!m || !f)
        return true;
      if (m->dead || f->dead)
        return true;

      // Change marriage status to divorced
      m->marriageStatus = MarriageStatus::Divorced;
      f->marriageStatus = MarriageStatus::Divorced;

      // Select who is to leave the household
      auto booted = (m->householdPosition == HouseholdPosition::Head) ? f : m;

      // Identify a new household for whoever left
      int newHouseholdID = -1;
      for (size_t i = 0; i < booted->offspring.size(); i++) {
        auto kid = booted->offspring[i].lock();
        if (!kid || kid->dead) continue;
        if (kid->householdID != m->householdID && \
            households[kid->householdID])
          newHouseholdID = kid->householdID;
      }

      auto mom = booted->mother.lock();
      if (mom && !mom->dead && mom->householdID != m->householdID && \
          households[mom->householdID != m->householdID])
        newHouseholdID = mom->householdID;

      auto dad = booted->father.lock();
      if (dad && !dad->dead && dad->householdID != m->householdID && \
          households[dad->householdID])
        newHouseholdID = dad->householdID;

      if (newHouseholdID > -1) {
        assert(households[newHouseholdID]);

        // If another household was found for the booted individual
        ChangeHousehold(booted, t, newHouseholdID, HouseholdPosition::Other);
      } else {
        Schedule(t, CreateHousehold(booted, weak_p<Individual>(), {}, {}));
      }

      data.divorces.Record(t, +1);
      return true;
    };

  return ef;
}
