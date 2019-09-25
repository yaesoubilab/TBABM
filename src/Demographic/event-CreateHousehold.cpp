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


// Algorithm S5: Create a household. All but the first argument may be empty pointers
//   or sets
EventFunc TBABM::CreateHousehold(weak_p<Individual> head_w,
    weak_p<Individual> spouse_w,
    std::vector<weak_p<Individual>> offspring_w,
    std::vector<weak_p<Individual>> other_w)
{
  EventFunc ef = 
    [this, head_w, spouse_w, offspring_w, other_w](double t, SchedulerT scheduler) {
      // printf("[%d] CreateHousehold\n", (int)t);
      long hid = nHouseholds++;

      auto head = head_w.lock();
      auto spouse = spouse_w.lock();

      vector<shared_p<Individual>> offspring{};
      vector<shared_p<Individual>> other{};

      for (auto idv : offspring_w)
        offspring.emplace_back(idv.lock());

      for (auto idv : other_w)
        other.emplace_back(idv.lock());

      auto household = std::make_shared<Household>(head, spouse, offspring, other, t, hid);
      households[hid] = household;

      ChangeHousehold(head, t, hid, HouseholdPosition::Head);
      ChangeHousehold(spouse, t, hid, HouseholdPosition::Spouse);

      for (auto idv : offspring)
        ChangeHousehold(idv, t, hid, HouseholdPosition::Offspring);

      for (auto idv : other)
        ChangeHousehold(idv, t, hid, HouseholdPosition::Offspring);

      return true;
    };

  return ef;
}
