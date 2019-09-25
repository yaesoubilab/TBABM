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

// Algorithm S3: Match making
EventFunc TBABM::Matchmaking(void)
{
  EventFunc ef = 
    [this](double t, SchedulerT scheduler) {

      // The maximum number of females who will be considered as 
      // potential spouses for each male. Currently implemented by
      // scanning from the beginning of the femaleSeeking vector.
      size_t under_consideration = 100;
      size_t scheduled_marriages = 0;

      auto male_it   = maleSeeking.begin();

      for (; male_it != maleSeeking.end();) {

        // Keep a weak and a shared pointer to the male being matched
        auto male_w = *male_it;
        auto male   = male_w.lock();

        // If the male is dead or something, erase them from the set,
        // and continue to the next male
        if (!male || male->dead) {
          male_it = maleSeeking.erase(male_it);
          continue;
        }

        // Obviously you can't match males up if there are no females
        if (femaleSeeking.size() == 0)
          break;

        // This stores the weights of each female being considered
        // for marriage to the current male being considered
        auto weights 			= std::vector<long double>{};
        long double denominator = 0;

        // For each female, compute a weight, corresponding to their
        // likelihood of marrying the male under consideration
        auto female_it = femaleSeeking.begin();
        for (size_t i = 0; 
            i < under_consideration && female_it != femaleSeeking.end();) {

          auto female_w = *female_it;
          auto female   = female_w.lock();

          // If the female is dead or something, erase them from the set,
          // and continue to the next female
          if (!female || female->dead) {
            female_it = femaleSeeking.erase(female_it);
            continue;
          }

          int maleAge   = (t - male->birthDate)   / 365;
          int femaleAge = (t - female->birthDate) / 365;

          auto ageDifference = std::abs(maleAge-femaleAge);

          // Calculate the weight of this pairing as a function of the age diff
          // between the pair under consideration, update the denominator
          // with this weight, and push the weight onto the 'weights' vector.
          long double weight = params["marriageAgeDifference"].pdf(ageDifference);
          denominator += weight;
          weights.push_back(weight);

          // Increment 'i' and the iterator. Note that this statement will not
          // run if the female has been discarded.
          i++;
          female_it++;

        }

        // Normalize the weights vector
        for (size_t i = 0; i < weights.size(); i++)
          weights[i] = weights[i] / denominator;

        // Create an empirical distribution on the possile pairings for 
        // this male. Sample from this distribution to determine which
        // pairing will be acted upon.
        auto wifeDist  = Empirical(weights);
        size_t wifeIdx = wifeDist(rng.mt_);
        auto wife_it   = femaleSeeking.begin();

        // Advance the iterator until it is pointing to the wife
        assert(wifeIdx >= 0);
        assert(wifeIdx < femaleSeeking.size());
        std::advance(wife_it, wifeIdx);

        // Get a weak pointer to the husband and wife
        auto wife_w    = *wife_it;
        auto husband_w = male_w;

        Schedule(t, Marriage(husband_w, wife_w));

        scheduled_marriages += 1;

        // Erase the wife from the pool of females so that she 
        // cannot be wed to another male in this matchmaking session
        femaleSeeking.erase(wife_it);

        male_it++;
      }

      // Erase males who have been matched to a female.
      maleSeeking.erase(maleSeeking.begin(), male_it);

      Schedule(t + 30, Matchmaking());

      return true;
    };


  return ef;
}
