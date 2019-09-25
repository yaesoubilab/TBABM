#include "../../include/TBABM/TBABM.h"
#include "../../include/TBABM/Individual.h"
#include "../../include/TBABM/SurveyUtils.h"

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

EventFunc TBABM::Survey(void)
{
  using MS  = MarriageStatus;

  EventFunc ef = 
    [this] (double t, SchedulerT scheduler) {
      string buf;
      string s = ",";

      printf("[%5d] Survey\n", (int)t);

      ///////////////////////////////////////////////////////
      // Population survey
      ///////////////////////////////////////////////////////
      for (auto it = population.begin(); it!= population.end(); it++) {
        auto idv = *it;

        if (!idv || idv->dead)
          continue;

        auto hh = households[idv->householdID];

        if (!hh || hh->size() == 0) continue;

        string line = to_string(seed) + s
          + to_string(t) + s
          + Ihash(idv) + s
          + age(idv, t) + s
          + sex(idv) + s
          + marital(idv) + s
          + to_string(households[idv->householdID]->size()) + s
          + Hhash(hh) + s
          + numChildren(idv) + s
          + mom(idv) + s
          + dad(idv) + s
          + HIV(idv) + s
          + ART(idv) + s
          + CD4(idv, t, params["HIV_m_30"].Sample(rng)) + s
          + TBStatus(idv, t)
          + "\n";

        buf += line;
      }

      populationSurvey += buf;
      buf.clear();

      ///////////////////////////////////////////////////////
      // Household survey
      ///////////////////////////////////////////////////////
      for (auto it = households.begin(); it != households.end(); it++) {
        auto hh = it->second;

        if (!hh || hh->size() == 0) continue;

        auto head =   hh->head;
        auto spouse = hh->spouse;

        int directOffspring {0};
        int otherOffspring  {0};

        for (auto i : hh->offspring)
          if (i->father.lock() == head || i->mother.lock() == head || \
              i->father.lock() == spouse || i->mother.lock() == spouse)
            directOffspring += 1;
          else
            otherOffspring += 1;

        string line = to_string(seed) 		      + s \
                      + to_string(t)    		      + s \
                      + Hhash(hh)		  		      + s \
                      + to_string(hh->size())       + s \
                      + to_string(!!hh->head)       + s \
                      + to_string(!!hh->spouse)     + s \
                      + to_string(directOffspring)  + s \
                      + to_string(otherOffspring)   + s \
                      + to_string(hh->other.size())     \
                      + "\n";

        buf += line;
      }

      householdSurvey += buf;

      Schedule(t + 15*365, Survey());

      return true;
    };

  return ef;
}

// data interested in:
// for each person:
//  - seed of trajectory they're in
//  - time
// 	- hash
// 	- age
// 	- sex
// 	- single, married, divorced, or looking
// 	- household size
// 	- household hash
// 	- number of children
// 	- mom alive?
// 	- dad alive?
// 	
// 	for each household:
// 	- trajectory
// 	- time
// 	- hash
// 	- size
// 	- head
// 	- spouse
// 	- directOffspring
// 	- otherOffspring
// 	- other
