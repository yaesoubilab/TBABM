#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <cmath>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace StatisticalDistributions;
using std::vector;
using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

// Algorithm S2: Create a population at simulation time 0
void TBABM::CreatePopulation(int t, long size)
{
  int popChange = 0;

  while (popChange < size) {
    long hid = nHouseholds++;
    shared_p<Household> hh = householdGen.GetHousehold(t, hid, rng);
    households[hid] = hh;

    assert(hh->head);

    // Unit: years
    double dt = constants["ageGroupWidth"] - fmod(hh->head->age<double>(t), constants["ageGroupWidth"]);

    // Insert all members of the household into the population
    population.push_back(hh->head); popChange++;
    assert(hh->head->householdID == hid);
    InitialEvents(hh->head, t, dt);
    Schedule(t + 365*dt, ChangeAgeGroup(hh->head));
    if (hh->spouse) {
      double dt = constants["ageGroupWidth"] - fmod(hh->spouse->age<double>(t), constants["ageGroupWidth"]);
      popChange++;
      population.push_back(hh->spouse);
      assert(hh->spouse->householdID == hid);
      InitialEvents(hh->spouse, t, dt);
      Schedule(t + dt, ChangeAgeGroup(hh->spouse));

      // Set marriage age
      double spouseAge = (t - hh->spouse->birthDate)/365.;
      double headAge = (t - hh->head->birthDate)/365.;
      hh->spouse->marriageDate = t - 365*fileData["timeInMarriage"].getValue(0,0,spouseAge,rng);
      hh->head->marriageDate   = t - 365*fileData["timeInMarriage"].getValue(0,0,headAge, rng);
    }
    for (auto it = hh->offspring.begin(); it != hh->offspring.end(); it++) {
      double dt = constants["ageGroupWidth"] - fmod((*it)->age<double>(t), constants["ageGroupWidth"]);
      popChange++;
      population.push_back((*it));
      assert((*it)->householdID == hid);
      Schedule(t + dt, ChangeAgeGroup(*it));
      InitialEvents(*it, t, dt);
    }
    for (auto it = hh->other.begin(); it != hh->other.end(); it++) {
      double dt = constants["ageGroupWidth"] - fmod((*it)->age<double>(t), constants["ageGroupWidth"]);
      popChange++;
      population.push_back((*it));
      assert((*it)->householdID == hid);
      Schedule(t + dt, ChangeAgeGroup(*it));

      if ((*it)->marriageStatus != MarriageStatus::Married &&
          params["otherMarried"].Sample(rng) == 1) {
        auto it2 = it;
        it2++;
        for (; it2 != hh->other.end(); it2++)
          if (*it2 &&
              !(*it2)->dead &&
              (*it)->sex != (*it2)->sex &&
              (*it2)->marriageStatus != MarriageStatus::Married) {

            auto male = (*it)->sex == Sex::Male ? *it : *it2;
            auto female = (*it)->sex == Sex::Male ? *it2 : *it;

            male->spouse = female;
            male->marriageStatus = MarriageStatus::Married;
            male->marriageDate = t - 365*fileData["timeInMarriage"].getValue(0,0,male->age(t),rng);

            female->spouse = male;
            female->marriageStatus = MarriageStatus::Married;
            female->marriageDate = t - 365*fileData["timeInMarriage"].getValue(0,0,female->age(t),rng);

            break;
          }
      }

      InitialEvents(*it, t, dt);
    }
  }

  // Decide whether each female is pregnant, and who has HIV. Schedule
  // HIV infection checks
  for (auto it = population.begin(); it != population.end(); it++) {
    auto person = *it;

    if (person->sex == Sex::Male || \
        !person->spouse.lock() || \
        person->age(t) < 15)
      continue;

    bool hasKids {person->offspring.size() > 0};

    auto birthDistribution = hasKids ? fileData["timeToSubsequentBirths"] : \
                             fileData["timeToFirstBirth"];

    double yearsToBirth = birthDistribution.getValue(0, 0, person->age(t), rng);
    int daysToFirstBirth = 365 * yearsToBirth;

    Schedule(t + daysToFirstBirth - 9*30, Pregnancy(person, person->spouse));
  }

  for (auto it = population.begin(); it != population.end(); it++) {

    auto person = *it;
    int gender = person->sex == Sex::Male ? 0 : 1;

    if (fileData["HIV_prevalence_1990"].getValue(1990, gender, person->age(t), rng) == 1)
      Schedule(t, HIVInfection(person));
    else
      Schedule(t + 365, HIVInfectionCheck(person));
  }

  for (auto it = population.begin(); it != population.end(); it++) {
    auto person = *it;
    int age = person->age(t);

    if (age < 15)
      data.populationChildren.Record(t, +1);
    else
      data.populationAdults.Record(t, +1);
  }

  data.populationSize.Record(t, popChange);
}
