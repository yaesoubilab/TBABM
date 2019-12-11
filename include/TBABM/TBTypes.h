#pragma once

#include <IncidenceTimeSeries.h>
#include <PrevalenceTimeSeries.h>
#include <PrevalencePyramidTimeSeries.h>
#include <DiscreteTimeStatistic.h>
#include "IndividualTypes.h"
#include "Pointers.h"

using Time = int;

class TB;

enum class TBStatus {
  Susceptible, Latent, Infectious
};

enum class TBTreatmentStatus {
  None, Incomplete, Complete, Dropout
};

enum class Source {Global, Household, Self};

enum class StrainType {Unspecified};

enum class RecoveryType {Natural, Treatment};

enum class CTraceType {None, All, Vul, IVul, Prob};

typedef struct TBHistoryItem {
  int t_infection;
  Source source;
  StrainType strain;

  TBHistoryItem(int t, 
      Source s, 
      StrainType strain) : t_infection(t), 
  source(s),
  strain(strain) {};
} TBHistoryItem;

typedef struct TBHandlers {
  function<void(int)> death;
} TBHandlers;

TBHandlers CreateTBHandlers(function<void(int)> death);

typedef struct TBQueryHandlers {
  function<int(Time)> Age;
  function<bool(void)> Alive;
  function<double(Time)> CD4Count;
  function<HIVStatus(void)> GetHIVStatus;
  function<bool(void)> ART;
  function<double(Time)> GlobalTBPrevalence;
  function<shared_p<TB>(void)> Lifetime;
} TBQueryHandlers;

TBQueryHandlers CreateTBQueryHandlers(
  function<int(Time)> Age,
  function<bool(void)> Alive,
  function<double(Time)> CD4Count,
  function<HIVStatus(void)> HIVStatus,
  function<bool(void)> ART,
  function<double(Time)> GlobalTBPrevalence,
  function<shared_p<TB>(void)> Lifetime
);

typedef IndividualSimContext TBSimContext; // For right now these are the same
