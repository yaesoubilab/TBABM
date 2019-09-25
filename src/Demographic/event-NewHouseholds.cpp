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

// Algorithm S4: Adding new residents
EventFunc TBABM::NewHouseholds(int num)
{
  EventFunc ef = 
    [this, num](double t, SchedulerT scheduler) {
      // printf("[%d] NewHouseholds\n", (int)t);
      int popChange = 0; // Number of people created so far

      while (popChange < num) {
        long hid = nHouseholds++;
        shared_p<Household> hh = householdGen.GetHousehold(t, hid, rng);
        households[hid] = hh;

        // Insert all members of the household into the population
        population.push_back(hh->head); popChange++;
        if (hh->spouse){
          population.push_back(hh->spouse);
          popChange++;
        }
        for (auto it = hh->offspring.begin(); it != hh->offspring.end(); it++) {
          popChange++;
          population.push_back(*it);
        }
        for (auto it = hh->other.begin(); it != hh->other.end(); it++) {
          popChange++;
          population.push_back(*it);
        }
      }

      data.populationSize.Record(t, popChange);

      return true;
    };

  return ef;
}
