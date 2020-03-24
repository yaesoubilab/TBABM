#pragma once

#include <boost/histogram.hpp>
#include <IncidenceTimeSeries.h>
#include <PrevalenceTimeSeries.h>
#include <IncidencePyramidTimeSeries.h>
#include <PrevalencePyramidTimeSeries.h>
#include <DiscreteTimeStatistic.h>

#include "IndividualTypes.h"
#include "TBTypes.h"

using namespace boost::histogram;

class MasterData {
  public:

    IncidenceTimeSeries<int> births;
    IncidenceTimeSeries<int> deaths;
    IncidenceTimeSeries<int> marriages;
    IncidenceTimeSeries<int> divorces;
    IncidenceTimeSeries<int> singleToLooking;

    PrevalenceTimeSeries<int> populationSize;
    PrevalenceTimeSeries<int> populationChildren;
    PrevalenceTimeSeries<int> populationAdults;

    IncidencePyramidTimeSeries pyramid;	
    IncidencePyramidTimeSeries deathPyramid;
    IncidenceTimeSeries<int> householdsCount;

    PrevalenceTimeSeries<int>   hivNegative;
    IncidenceTimeSeries<int>    hivInfections;
    PrevalenceTimeSeries<int>   hivPositive;
    PrevalenceTimeSeries<int>   hivPositiveART;
    PrevalenceTimeSeries<int>   hivDiagnosed;
    PrevalenceTimeSeries<int>   hivDiagnosedVCT;
    IncidenceTimeSeries<int>    hivDiagnosesVCT;
    PrevalencePyramidTimeSeries hivPositivePyramid;
    IncidencePyramidTimeSeries  hivInfectionsPyramid;

    IncidenceTimeSeries<int>  tbInfections;  // Individuals transitioning from S to L
    IncidenceTimeSeries<int>  tbIncidence;   // Individuals transitioning from L to I
    IncidenceTimeSeries<int>  tbRecoveries;  // Individuals transitioning from I to L

    // Individuals infected by household member (transitioning from L to I)
    IncidenceTimeSeries<int>  tbInfectionsHousehold; 
    
    // Individuals infected by community (transitioning from L to I)
    IncidenceTimeSeries<int>  tbInfectionsCommunity; 

    PrevalenceTimeSeries<int> tbSusceptible; // # Individuals in S
    PrevalenceTimeSeries<int> tbLatent;      // # Individuals in L
    PrevalenceTimeSeries<int> tbInfectious;  // # Individuals in I

    PrevalenceTimeSeries<int> tbExperienced; // # Individuals who are experienced with TB (L or I)
    PrevalencePyramidTimeSeries tbExperiencedPyr; // Pyramid of the above

    IncidenceTimeSeries<int>  tbTreatmentBegin;   // Individuals initiating treatment
    IncidenceTimeSeries<int> tbTreatmentBeginHIV;// Initiating but also HIV+
    IncidenceTimeSeries<int> tbTreatmentBeginChildren;
    IncidenceTimeSeries<int> tbTreatmentBeginAdultsNaive;
    IncidenceTimeSeries<int> tbTreatmentBeginAdultsExperienced;
    IncidenceTimeSeries<int>  tbTreatmentEnd;     // Individuals completing treatment
    IncidenceTimeSeries<int>  tbTreatmentDropout; // Individuals dropping out

    PrevalenceTimeSeries<int> tbInTreatment;        // Individuals in treatment
    PrevalenceTimeSeries<int> tbCompletedTreatment; // Individuals who completed
    PrevalenceTimeSeries<int> tbDroppedTreatment;   // Individuals who dropped

    PrevalenceTimeSeries<int> tbTxExperiencedAdults;
    PrevalenceTimeSeries<int> tbTxExperiencedInfectiousAdults;
    PrevalenceTimeSeries<int> tbTxNaiveAdults;
    PrevalenceTimeSeries<int> tbTxNaiveInfectiousAdults;

    IncidenceTimeSeries<int>  tbDeaths;
    IncidenceTimeSeries<int>  tbDeathsHIV;
    IncidenceTimeSeries<int>  tbDeathsUnderFive;

    IncidenceTimeSeries<int> ctHomeVisits;
    IncidenceTimeSeries<int> ctScreenings;
    IncidenceTimeSeries<int> ctScreeningsHIV;
    IncidenceTimeSeries<int> ctScreeningsChildren;
    IncidenceTimeSeries<int> ctCasesFound;
    IncidenceTimeSeries<int> ctCasesFoundHIV;
    IncidenceTimeSeries<int> ctCasesFoundChildren;
    IncidenceTimeSeries<int> ctDeathsAverted;
    IncidenceTimeSeries<int> ctDeathsAvertedHIV;
    IncidenceTimeSeries<int> ctDeathsAvertedChildren;
    HistT                    ctInfectiousnessAverted = make_histogram(axis::regular<>(12,0,365));

    IncidenceTimeSeries<int> activeHouseholdContacts;
    IncidenceTimeSeries<int> activeHouseholdContactsUnder5;
    IncidenceTimeSeries<int> totalHouseholdContacts;
    IncidenceTimeSeries<int> totalHouseholdContactsUnder5;

    MasterData(int tMax, int pLength, std::vector<double> ageBreaks);

    void Close(void);
    
  private:
};
