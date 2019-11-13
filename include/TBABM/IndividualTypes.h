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
