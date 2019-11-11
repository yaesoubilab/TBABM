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

typedef struct TBData {
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

  DiscreteTimeStatistic& activeHouseholdContacts; // For each individual diagnosed with active TB,
  // the percentage of household contacts who have
  // active TB.
} TBData;

TBData CreateTBData(IndividualInitData data);

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
