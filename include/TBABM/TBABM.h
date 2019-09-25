#pragma once

#include <map>
#include <string>
#include <ctime>
#include <fstream>
#include <iostream>

#include <StatisticalDistribution.h>
#include <Exponential.h>
#include <RNG.h>
#include <EventQueue.h>
#include <PrevalenceTimeSeries.h>
#include <IncidenceTimeSeries.h>
#include <IncidencePyramidTimeSeries.h>
#include <PrevalencePyramidTimeSeries.h>
#include <CSVExport.h>
#include <EventQueue.h>

#include <Param.h>
#include <DataFrame.h>
#include <JSONImport.h>

#include "MasterData.h"

#include "Individual.h"
#include "IndividualTypes.h"
#include "Names.h"

#include "Household.h"
#include "HouseholdGen.h"

#include "Pointers.h"

#include "utils/termcolor.h"

using std::map;
using std::vector;
using std::string;

using namespace StatisticalDistributions;
using namespace SimulationLib;
using namespace SimulationLib::JSONImport;

class TBABM {
  public:
    using Params = map<string, Param>;
    using Constants = map<string, long double>;

    using EQ = EventQueue<double, bool>;
    using EventFunc = EQ::EventFunc;
    using SchedulerT = EQ::SchedulerT;

    TBABM(Params params_, 
        std::map<string, long double> constants_,
        const char *householdsFile, 
        const std::uint_fast64_t _seed) : 

      params(params_),
      constants(constants_),

      data(constants_["tMax"],
          constants_["periodLength"],
          {15, 25, 35, 45, 55, 65}),

      seed(_seed),
      rng(_seed),

      householdGen(householdsFile, 
          params,
          fileData,
          eq,
          [this] (void) { return data.GenIndividualInitData(); },
          CreateIndividualHandlers([this] (weak_ptr<Individual> i, int t, DeathCause dc) -> void { return Schedule(t, Death(i, dc)); },
            [this] (int t) -> double { return (double)data.tbInfectious(t)/(double)data.populationSize(t); }))
      {
        // Associate DataFrameFile's with parameters which map to a file, rather
        // than directly to a distribution
        for (auto it = params.begin(); it != params.end(); it++)
          if (it->second.getType() == SimulationLib::Type::file_type) {
            auto j = JSONImport::fileToJSON(it->second.getFileName());
            fileData[it->first] = DataFrameFile{j};
          }

        printf("Seed: %llu\n", seed);
      };

      bool Run(void);

      MasterData
        GetData(void);

      bool WriteSurveys(shared_p<ofstream> ps, 
          shared_p<ofstream> hs, 
          shared_p<ofstream> ds);

  private:

      ////////////////////////////////////////////////////////
      /// Demographic Events
      ////////////////////////////////////////////////////////

      // Algorithm S2: Create a population at simulation time 0
      void CreatePopulation(int t, long size);

      // Algorithm S3: Match making
      EventFunc Matchmaking(void);

      // Algorithm S4: Adding new residents
      EventFunc NewHouseholds(int num);

      // Algorithm S5: Create a household
      EventFunc CreateHousehold(weak_p<Individual> head,
          weak_p<Individual> spouse,
          std::vector<weak_p<Individual>> offspring,
          std::vector<weak_p<Individual>> other);

      // Algorithm S6: Birth
      EventFunc Birth(weak_p<Individual> mother, weak_p<Individual> father);

      // Algorithm S7: Joining a household
      EventFunc JoinHousehold(weak_p<Individual>, long hid);

      // Algorithm S9: Change of age groups
      EventFunc ChangeAgeGroup(weak_p<Individual>);

      // Algorithm S10: Natural death
      EventFunc Death(weak_p<Individual>, DeathCause deathCause);

      // Algorithm S11: Leave current household to form new household
      EventFunc LeaveHousehold(weak_p<Individual>);

      // Algorithm S12: Change of marital status from single to looking
      EventFunc SingleToLooking(weak_p<Individual>);

      // Algorithm S13: Marriage
      EventFunc Marriage(weak_p<Individual> m, weak_p<Individual> f);

      // Algorithm S14: Divorce
      EventFunc Divorce(weak_p<Individual> m, weak_p<Individual> f);

      EventFunc Pregnancy(weak_p<Individual> f, weak_p<Individual> m);

      // Update the population pyramid
      EventFunc UpdatePyramid(void);

      // Update households
      EventFunc UpdateHouseholds(void);

      // Population survey
      EventFunc Survey(void);

      // BYPASS for debugging
      EventFunc ExogenousBirth(void);

      ////////////////////////////////////////////////////////
      /// Demographic Utilities
      ////////////////////////////////////////////////////////
      void InitialEvents(weak_p<Individual> idv, double t, double dt);

      void PurgeReferencesToIndividual(weak_p<Individual> host,
          weak_p<Individual> idv);

      void DeleteIndividual(weak_p<Individual> idv);

      void ChangeHousehold(weak_p<Individual> idv, int time, int newHID, HouseholdPosition newRole);

      void SurveyDeath(shared_p<Individual> idv, int t, DeathCause deathCause);

      Names name_gen;

      ////////////////////////////////////////////////////////
      /// HIV Events
      ////////////////////////////////////////////////////////
      EventFunc ARTGuidelineChange(void);
      EventFunc ARTInitiate(weak_p<Individual>);
      EventFunc HIVInfectionCheck(weak_p<Individual>);
      EventFunc HIVInfection(weak_p<Individual>);
      EventFunc MortalityCheck(weak_p<Individual>);
      EventFunc VCTDiagnosis(weak_p<Individual>);

      ////////////////////////////////////////////////////////
      /// HIV Utilities
      ////////////////////////////////////////////////////////

      bool ARTEligible(int t, weak_p<Individual> idv);
      void HIVInfectionCheck(int t, weak_p<Individual> idv);

      ////////////////////////////////////////////////////////
      /// Scheduling
      ////////////////////////////////////////////////////////
      EventQueue<> eq;

      void Schedule(int t, EventQueue<>::EventFunc ef);

      ////////////////////////////////////////////////////////
      /// Data
      ////////////////////////////////////////////////////////

      map<string, DataFrameFile> fileData;
      Params params;

      MasterData data;

      vector<shared_p<Individual>> population;
      map<long, shared_p<Household>> households;

      vector<weak_p<Individual>> maleSeeking;
      vector<weak_p<Individual>> femaleSeeking;

      vector<weak_p<Individual>> seekingART; // Individuals seeking ART

      Constants constants;

      HouseholdGen householdGen;

      long nHouseholds = 0;

      RNG rng;
      std::uint_fast64_t seed;

      string populationSurvey;
      string householdSurvey;
      string deathSurvey;
};
