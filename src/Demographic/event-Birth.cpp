#include "../../include/TBABM/TBABM.h"
#include <cassert>

using std::vector;
using EventFunc = TBABM::EventFunc;
using SchedulerT = EQ::SchedulerT;

// Algorithm S6: Birth
EventFunc TBABM::Birth(weak_p<Individual> mother_w, weak_p<Individual> father_w)
{
  EventFunc ef = 
    [this, mother_w, father_w](double t, SchedulerT scheduler) {
      auto mother = mother_w.lock();
      auto father = father_w.lock();

      // If mother is dead
      if (!mother || mother->dead)
        return true;

      mother->pregnant = false;

      // Decide properties of baby
      Sex sex = params["sex"].Sample(rng) ?
        Sex::Male : Sex::Female;

      HouseholdPosition householdPosition = HouseholdPosition::Offspring;
      MarriageStatus marriageStatus = MarriageStatus::Single;

      auto deathHandler = [this] (weak_p<Individual> idv, int t, DeathCause cause) -> void { 
        return Schedule(t, Death(idv, cause));
      };

      auto GlobalTBHandler = [this] (int t) -> double {
        return (double)data.tbInfectious(t)/(double)data.populationSize(t);
      };

      // Construct baby
      auto baby = makeIndividual(
          CreateIndividualSimContext(t, eq, rng, fileData, params),
          data,
          CreateIndividualHandlers(deathHandler, GlobalTBHandler),
          name_gen.getName(rng),
          mother->householdID, t, sex,
          weak_p<Individual>(), mother_w, father_w,
          vector<weak_p<Individual>>{}, householdPosition, marriageStatus);

      // printf("[%d] Baby born: %ld::%lu\n", (int)t, mother->householdID, std::hash<Pointer<Individual>>()(baby));

      // Add baby to household and population
      mother->offspring.push_back(baby);
      if (father && !father->dead)
        father->offspring.push_back(baby);

      population.push_back(baby);

      ChangeHousehold(baby, t, mother->householdID, householdPosition);

      Schedule(t, ChangeAgeGroup(baby));

      data.populationSize.Record(t, +1);
      data.populationChildren.Record(t, +1);
      data.births.Record(t, +1);

      // Schedule the next birth
      auto yearsToNextBirth = fileData["timeToSubsequentBirths"].getValue(0,0,(t-mother->birthDate)/365,rng);
      auto daysToNextBirth = 365*yearsToNextBirth;

      Schedule(t + daysToNextBirth - 9*30, Pregnancy(mother, mother->spouse));

      return true;
    };

  return ef;
}
