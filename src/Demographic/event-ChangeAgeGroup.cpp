#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace StatisticalDistributions;
using std::vector;
using std::map;
using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

auto findHousehold = [] (map<long, shared_p<Household>> &households) -> long {
  for (auto household : households)
    if (household.second && 
        household.second->size() > 0 &&
        household.second->size() < 5)
      return household.first;

  return -1;
};

// Algorithm S9: Change of age groups
EventFunc TBABM::ChangeAgeGroup(weak_p<Individual> idv_w)
{
  int ageGroupWidth = constants["ageGroupWidth"]; // Age groups go 0-5, 5-10, etc.

  EventFunc ef = 
    [this, idv_w, ageGroupWidth](double t, SchedulerT scheduler) {
      auto idv = idv_w.lock();
      if (!idv)
        return true;
      if (idv->dead)
        return true;

      double timeToNextEvent = ageGroupWidth*365;

      ////////////////////////////////////////////////////////
      /// Natural death
      ////////////////////////////////////////////////////////
      auto startYear = constants["startYear"];
      int gender = idv->sex == Sex::Male ? 0 : 1;
      double age = idv->age(t);
      double timeToDeath = 365 * fileData["naturalDeath"].getValue(startYear+(int)t/365, gender, age, rng);

      bool scheduledDeath = false;
      if (timeToDeath < timeToNextEvent) {
        scheduledDeath = true;
        Schedule(t + timeToDeath, Death(idv, DeathCause::Natural));
      }

      //////////////////////////////////////////////////////
      // Leaving the current household to form a new household
      //////////////////////////////////////////////////////
      bool idvIsHead = idv->householdPosition == HouseholdPosition::Head;
      double timeToLeave = 365*params["leavingHousehold"].Sample(rng);
      if (!idv->spouse.lock() && 
          age >= 18 && 
          age <= 55 && 
          !idvIsHead && 
          timeToLeave < timeToNextEvent &&
          timeToLeave < timeToDeath) {
        Schedule(t + timeToLeave, LeaveHousehold(idv));
      }

      ////////////////////////////////////////////////////
      // Change of marital status from single to looking
      ////////////////////////////////////////////////////
      double scale  = params["timeToLookingScale"].Sample(rng);
      double sample = fileData["timeToLooking"].getValue(0,gender,(t-idv->birthDate)/365,rng);
      double timeToLook = 365 * scale * sample;
      if ((idv->marriageStatus == MarriageStatus::Single ||
            idv->marriageStatus == MarriageStatus::Divorced)
          && timeToLook < timeToNextEvent
          && timeToLook < timeToDeath) {
        Schedule(t + timeToLook, SingleToLooking(idv));
      }

      //////////////////////////////////////////////////////
      // Joining a household
      //////////////////////////////////////////////////////
      auto household = households[idv->householdID];
      assert(household);

      bool changed {false};
      if (age >= 65 && household->size() == 1) {
        for (size_t i = 0; i < idv->livedWithBefore.size(); i++) {
          auto person = idv->livedWithBefore[i].lock();
          if (!person || person->dead)
            continue;

          int hid = person->householdID;

          if (!households[hid])
            continue;

          ChangeHousehold(idv, t, hid, HouseholdPosition::Other);
          changed = true;
          break;
        }
        if (!changed)
          ChangeHousehold(idv, t, findHousehold(households), HouseholdPosition::Other);
      }

      if (!scheduledDeath)
        Schedule(t + timeToNextEvent, ChangeAgeGroup(idv));


      // Schedule(t, HIVInfectionCheck(idv));

      return true;
    };

  return ef;
}
