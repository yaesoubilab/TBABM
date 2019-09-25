#include "../include/TBABM/MasterData.h"

MasterData::MasterData(int tMax, int pLength, std::vector<double> ageBreaks) :
  births(         "births",          0, tMax, pLength),
  deaths(         "deaths",          0, tMax, pLength),
  marriages(      "marriages",       0, tMax, pLength),
  divorces(       "divorces",        0, tMax, pLength),
  singleToLooking("singleToLooking", 0, tMax, pLength),

  populationSize( "populationSize",     tMax, pLength),
  populationChildren("populationChildren", tMax, pLength),
  populationAdults(  "populationAdults", tMax, pLength),

  hivNegative(   "hivNegative",    tMax, pLength),
  hivPositive(   "hivPositive",    tMax, pLength),
  hivPositiveART("hivPositiveART", tMax, pLength),
  hivPositivePyramid("hivPositive pyramid", 0, tMax, pLength, 2, ageBreaks),

  hivInfectionsPyramid("hivInfections pyramid", 0, tMax, pLength, 2, ageBreaks),
  hivInfections(  "hivInfections", 0,   tMax, pLength),
  hivDiagnosed(   "hivDiagnosed",       tMax, pLength),
  hivDiagnosedVCT("hivDiagnosedVCT",    tMax, pLength),
  hivDiagnosesVCT("hivDiagnosesVCT", 0, tMax, pLength),

  tbInfections( "tbInfections",  0, tMax, pLength),
  tbIncidence(  "tbIncidence"  , 0, tMax, pLength),
  tbRecoveries( "tbRecoveries",  0, tMax, pLength),

  tbInfectionsHousehold( "tbInfectionsHousehold",  0, tMax, pLength),
  tbInfectionsCommunity( "tbInfectionsCommunity",  0, tMax, pLength),

  tbSusceptible("tbSusceptible", tMax, pLength),
  tbLatent(     "tbLatent",      tMax, pLength),
  tbInfectious( "tbInfectious",  tMax, pLength),

  tbExperienced("tbExperienced", tMax, pLength),
  tbExperiencedPyr("tbExperiencedPyr", 0, tMax, pLength, 2, ageBreaks),

  tbTreatmentBegin(   "tbTreatmentBegin",   0, tMax, pLength),
  tbTreatmentBeginHIV("tbTreatmentBeginHIV",0, tMax, pLength),
  tbTreatmentBeginChildren("tbTreatmentBeginChildren",0, tMax, pLength),
  tbTreatmentBeginAdultsNaive("tbTreatmentBeginAdultsNaive",0, tMax, pLength),
  tbTreatmentBeginAdultsExperienced("tbTreatmentBeginAdultsExperienced",0, tMax, pLength),
  tbTreatmentEnd(     "tbTreatmentEnd",     0, tMax, pLength),
  tbTreatmentDropout( "tbTreatmentDropout", 0, tMax, pLength),

  tbInTreatment(       "tbInTreatment",        tMax, pLength),
  tbCompletedTreatment("tbCompletedTreatment", tMax, pLength),
  tbDroppedTreatment(  "tbDroppedTreatment",   tMax, pLength),

  tbTxExperiencedAdults("tbTxExperiencedAdults", tMax, pLength),
  tbTxExperiencedInfectiousAdults("tbTxExperiencedInfectiousAdults", tMax, pLength),
  tbTxNaiveAdults("tbTxNaiveAdults", tMax, pLength),
  tbTxNaiveInfectiousAdults("tbTxNaiveInfectiousAdults", tMax, pLength),

  activeHouseholdContacts("activeHouseholdContacts"),

  pyramid("Population pyramid", 0, tMax, pLength, 2, ageBreaks),
  deathPyramid("Death pyramid", 0, tMax, pLength, 2, ageBreaks),
  householdsCount("households", 0, tMax, pLength)
{}

  IndividualInitData
MasterData::GenIndividualInitData(void)
{
  return CreateIndividualInitData(
      tbInfections,
      tbIncidence,
      tbRecoveries,
      tbInfectionsHousehold,
      tbInfectionsCommunity,
      tbSusceptible,
      tbLatent,
      tbInfectious,
      tbExperienced,
      tbExperiencedPyr,
      tbTreatmentBegin,
      tbTreatmentBeginHIV,
      tbTreatmentBeginChildren,
      tbTreatmentBeginAdultsNaive,
      tbTreatmentBeginAdultsExperienced,
      tbTreatmentEnd,
      tbTreatmentDropout,
      tbInTreatment,
      tbCompletedTreatment,
      tbDroppedTreatment,
      tbTxExperiencedAdults,
      tbTxExperiencedInfectiousAdults,
      tbTxNaiveAdults,
      tbTxNaiveInfectiousAdults,
      activeHouseholdContacts
        );
}

  void
MasterData::Close(void)
{
  births.Close();
  deaths.Close();
  marriages.Close();
  divorces.Close();
  singleToLooking.Close();

  populationSize.Close();
  populationChildren.Close();
  populationAdults.Close();

  pyramid.Close();
  deathPyramid.Close();
  householdsCount.Close();

  hivNegative.Close();
  hivInfections.Close();
  hivPositive.Close();
  hivPositiveART.Close();
  hivDiagnosed.Close();
  hivDiagnosedVCT.Close();
  hivDiagnosesVCT.Close();
  hivPositivePyramid.Close();
  hivInfectionsPyramid.Close();

  tbInfections.Close();
  tbIncidence.Close();
  tbRecoveries.Close();
  tbInfectionsHousehold.Close();
  tbInfectionsCommunity.Close();
  tbSusceptible.Close();
  tbLatent.Close();
  tbInfectious.Close();
  tbExperienced.Close();
  tbExperiencedPyr.Close();

  tbTreatmentBegin.Close();
  tbTreatmentBeginHIV.Close();
  tbTreatmentBeginChildren.Close();
  tbTreatmentBeginAdultsNaive.Close();
  tbTreatmentBeginAdultsExperienced.Close();
  tbTreatmentEnd.Close();
  tbTreatmentDropout.Close();

  tbTxExperiencedAdults.Close();
  tbTxExperiencedInfectiousAdults.Close();
  tbTxNaiveAdults.Close();
  tbTxNaiveInfectiousAdults.Close();

  tbInTreatment.Close();
  tbCompletedTreatment.Close();
  tbDroppedTreatment.Close();

  return;
}
