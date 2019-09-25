#include "../../include/TBABM/TBABM.h"
#include "../../include/TBABM/SurveyUtils.h"
#include "../../include/TBABM/utils/termcolor.h"
#include <cassert>
#include <fstream>
#include <iostream>

using namespace StatisticalDistributions;
using std::vector;
using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

// Algorithm S10: Natural death
EventFunc TBABM::Death(weak_p<Individual> idv_w, DeathCause deathCause)
{
  EventFunc ef = 
    [this, idv_w, deathCause](double t, SchedulerT scheduler) {

      // Be sure this individual is alive
      auto idv = idv_w.lock();
      if (!idv)
        return true;
      if (idv->dead)
        return true;

      SurveyDeath(idv, t, deathCause);

      // Look up household, and assert that this household actually exists
      auto household = households[idv->householdID];
      assert(household);

      // Eliminate from Looking pools
      if (idv->sex == Sex::Male) {
        for (auto it = maleSeeking.begin(); it != maleSeeking.end(); it++)
          if (it->lock() == idv) {
            maleSeeking.erase(it);
            break;
          }
      } else {
        for (auto it = femaleSeeking.begin(); it != femaleSeeking.end(); it++)
          if (it->lock() == idv) {
            femaleSeeking.erase(it);
            break;
          }
      }

      // Advise spouse that they are now widowed
      if (auto spouse = idv->spouse.lock())
        spouse->Widowed();

      int age = idv->age(t);
      int sex = idv->sex == Sex::Male ? 0 : 1;

      idv->dead = true;

      // This purges 'idv' from the 'livedWithBefore' records
      // of all who have lived with 'idv'
      DeleteIndividual(idv);

      // Removes the individual from the household and elects a 
      // new head, if 'idv' was the head of his or her household
      household->RemoveIndividual(idv, t);

      // The call to 'RemoveIndivdual' could cause a house to no 
      // longer have any members in it. In this case, remove the
      // household
      if (household->size() == 0)
        households[idv->householdID].reset();

      // With 'idv' cut out of others records, and their household,
      // it is now safe to erase them from the population
      for (auto it = population.begin(); it != population.end(); it++)
        if (*it == idv) {
          population.erase(it);
          break;
        }

      // Advise TB object that individual has died
      idv->tb.HandleDeath(t);

      // Update various metrics
      data.populationSize.Record(t, -1);
      data.deaths.Record(t, +1);
      data.deathPyramid.UpdateByAge(t, sex, age, +1);

      if (age < 15)
        data.populationChildren.Record(t, -1);
      else
        data.populationAdults.Record(t, -1);

      if (idv->hivStatus == HIVStatus::Positive)
        data.hivPositive.Record(t, -1);

      if (idv->hivStatus == HIVStatus::Positive &&
          idv->onART)
        data.hivPositiveART.Record(t, -1);

      if (idv->hivStatus == HIVStatus::Positive &&
          idv->hivDiagnosed) {
        data.hivDiagnosed.Record(t, -1);
        data.hivDiagnosedVCT.Record(t, -1);
      }

      return true;
    };

  return ef;
}

void TBABM::SurveyDeath(shared_p<Individual> idv, int t, DeathCause deathCause)
{
  string s = ",";

  string line = to_string(seed) + s
    + to_string(t) + s
    + Ihash(idv) + s
    + age(idv, t) + s
    + sex(idv) + s
    + causeDeath(deathCause) + s
    + HIV(idv) + s 
    + HIV_date(idv) + s 
    + ART(idv) + s 
    + ART_date(idv) + s 
    + CD4(idv, t, params["HIV_m_30"].Sample(rng)) + s
    + ART_baseline_CD4(idv, params["HIV_m_30"].Sample(rng))
    + "\n";

  deathSurvey += line;

  return;
}

// - trajectory
// - time: in days
// - hash
// - age: in years
// - sex: male/female
// - cause: HIV/natural
// - HIV: true/false
// - HIV_date: in days
// - ART: true/false
// - ART_date: in days
// - CD4
// - baseline CD4 (CD4 at time of ART initiation)
