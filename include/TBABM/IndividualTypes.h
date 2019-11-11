#pragma once

#include <boost/histogram.hpp>
#include <IncidenceTimeSeries.h>
#include <PrevalenceTimeSeries.h>
#include <PrevalencePyramidTimeSeries.h>
#include <DiscreteTimeStatistic.h>
#include <EventQueue.h>
#include <RNG.h>
#include <Param.h>
#include <DataFrame.h>
#include "Pointers.h"

using namespace boost::histogram;
using namespace SimulationLib;
using StatisticalDistributions::RNG;
using std::function;
using std::map;
using EQ = EventQueue<double, bool>;
using Params = map<string, Param>;

using Time = int;

class Individual;

enum class Sex {
  Male, Female
};

enum class HouseholdPosition {
  Head, Spouse, Offspring, Other
};

enum class MarriageStatus {
  Single, Married, Divorced, Looking
};

enum class HIVStatus {
  Negative, Positive
};

enum class DeathCause {
  Natural, HIV, TB
};

using HistT = decltype(make_histogram(axis::regular<>(12,0,365)));

typedef struct IndividualInitData {
  IncidenceTimeSeries<int>& tbInfections;  // Individuals transitioning from S to L
  IncidenceTimeSeries<int>& tbIncidence;   // Individuals transitioning from L to I
  IncidenceTimeSeries<int>& tbRecoveries;  // Individuals transitioning from I to L

  IncidenceTimeSeries<int>& tbInfectionsHousehold; // Individuals infected by household member
  IncidenceTimeSeries<int>& tbInfectionsCommunity; // Individuals infected by community

  PrevalenceTimeSeries<int>& tbSusceptible; // # Individuals in S
  PrevalenceTimeSeries<int>& tbLatent;      // # Individuals in L
  PrevalenceTimeSeries<int>& tbInfectious;  // # Individuals in I
  PrevalenceTimeSeries<int>& tbExperienced; // # Individuals who are experienced with TB (L or I)

  PrevalencePyramidTimeSeries& tbExperiencedPyr; // Pyramid of the above

  IncidenceTimeSeries<int>& tbTreatmentBegin;   // Individuals initiating treatment
  IncidenceTimeSeries<int>& tbTreatmentBeginHIV;// Initiating treatment and HIV+
  IncidenceTimeSeries<int>& tbTreatmentBeginChildren;
  IncidenceTimeSeries<int>& tbTreatmentBeginAdultsNaive;
  IncidenceTimeSeries<int>& tbTreatmentBeginAdultsExperienced;
  IncidenceTimeSeries<int>& tbTreatmentEnd;     // Individuals completing treatment
  IncidenceTimeSeries<int>& tbTreatmentDropout; // Individuals dropping out

  PrevalenceTimeSeries<int>& tbInTreatment;        // Individuals in treatment
  PrevalenceTimeSeries<int>& tbCompletedTreatment; // Individuals who completed
  PrevalenceTimeSeries<int>& tbDroppedTreatment;   // Individuals who dropped

  PrevalenceTimeSeries<int>& tbTxExperiencedAdults;
  PrevalenceTimeSeries<int>& tbTxExperiencedInfectiousAdults;
  PrevalenceTimeSeries<int>& tbTxNaiveAdults;
  PrevalenceTimeSeries<int>& tbTxNaiveInfectiousAdults;

  IncidenceTimeSeries<int>& ctHomeVisits;
  IncidenceTimeSeries<int>& ctScreenings;
  IncidenceTimeSeries<int>& ctCasesFound;
  IncidenceTimeSeries<int>& ctDeathsAverted;
  HistT&                    ctInfectiousnessAverted;

  DiscreteTimeStatistic& activeHouseholdContacts; // For each individual diagnosed with active TB,
  // the percentage of household contacts who have
  // active TB.
} IndividualInitData;

  template <typename... Ts>
IndividualInitData CreateIndividualInitData(Ts&&... args)
{
  return { std::forward<Ts>(args)... };
}

typedef struct IndividualHandlers {
  function<void(weak_p<Individual>, int, DeathCause)> Death;
  function<double(int)> GlobalTBPrevalence;
} IndividualHandlers;

IndividualHandlers CreateIndividualHandlers(
    function<void(weak_p<Individual>, int, DeathCause)> Death,
    function<double(int)> GlobalTBPrevalence
    );

typedef struct IndividualSimContext {
  int current_time;
  EQ& event_queue;
  RNG &rng;
  map<string, DataFrameFile>& fileData;
  Params& params;
} IndividualSimContext;

IndividualSimContext CreateIndividualSimContext(
    int current_time, 
    EQ& event_queue, 
    RNG &rng,
    map<string, DataFrameFile>& fileData,
    Params& params
);
