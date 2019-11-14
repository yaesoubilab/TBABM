#pragma once

#include <cassert>

#include <RNG.h>
#include <Param.h>

#include "Individual.h"
#include "IndividualTypes.h"
#include "HouseholdTypes.h"
#include "TBTypes.h"
#include "Pointers.h"

class Household {
  public:
    shared_p<Individual> head;
    shared_p<Individual> spouse;
    std::vector<shared_p<Individual>> offspring;
    std::vector<shared_p<Individual>> other;

    void AddIndividual(shared_p<Individual> idv, int t, HouseholdPosition hp);

    void RemoveIndividual(weak_p<Individual> idv, int t);

    void PrintHousehold(int t);

    // Right now, these functions mean the same thing,
    // and 't' is not used.
    int size(int t);
    int size(void);

    bool hasMember(weak_p<Individual> idv);

    // ActiveTBPrevalence is simply the fraction of individuals
    // in the household who have active TB. Right now,
    // ActiveTBPrevalence(1) defers to ActiveTBPrevalence().
    double ActiveTBPrevalence(void);
    double ActiveTBPrevalence(int t);

    // ContactActiveTBPrevalence returne the fraction of household
    // contacts (excluding the individual with the passed-in TBStatus)
    // who have active TB.
    double ContactActiveTBPrevalence(TBStatus);
    double ContactActiveTBPrevalence(TBStatus, int t);

    // Returns true iff one of the members of the household is HIV+ or <5yo
    bool HasVulnerable(int t);

    // Do a contact trace on the household. Returns the number of cases
    // of active, untreated TB in the household (NOT including the 
    // individual causing the tracing event to occur)
    ContactTraceResult
    ContactTrace(const int& t,
                 const shared_p<Individual> idv,
                 Param& frac_screened,
                 RNG& rng);

    Household(shared_p<Individual> head,
        shared_p<Individual> spouse,
        std::vector<shared_p<Individual>> offspring,
        std::vector<shared_p<Individual>> other,
        int t,
        int hid) : 
      head(head), 
      spouse(spouse), 
      offspring(offspring), 
      other(other),
      hid(hid),
      nInfectiousTBIndivduals(0) {

        nIndividuals = (head?1:0) + 
          (spouse?1:0) + 
          offspring.size() + 
          other.size();

        AddIndividual(head, t, HouseholdPosition::Head);
        AddIndividual(spouse, t, HouseholdPosition::Spouse);

        for (auto idv : offspring)
          AddIndividual(idv, t, HouseholdPosition::Offspring);
        for (auto idv : other)
          AddIndividual(idv, t, HouseholdPosition::Other);
      }

    Household(int t, int hid) : 
      Household({}, {}, {}, {}, t, hid) {}

  private:
    int hid;
    int nIndividuals;
    int nInfectiousTBIndivduals;

    void TriggerReeval(int t, weak_p<Individual> idv);
};
