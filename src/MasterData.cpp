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

  tbDeaths("tbDeaths", 0, tMax, pLength),
  tbDeathsHIV("tbDeathsHIV", 0, tMax, pLength),
  tbDeathsUnderFive("tbDeathsUnderFive", 0, tMax, pLength),

  ctHomeVisits("ctHomeVisits", 0, tMax, pLength),
  ctScreenings("ctScreenings", 0, tMax, pLength),
  ctScreeningsHIV("ctScreeningsHIV", 0, tMax, pLength),
  ctScreeningsChildren("ctScreeningsChildren", 0, tMax, pLength),
  ctCasesFound("ctCasesFound", 0, tMax, pLength),
  ctCasesFoundHIV("ctCasesFoundHIV", 0, tMax, pLength),
  ctCasesFoundChildren("ctCasesFoundChildren", 0, tMax, pLength),
  ctDeathsAverted("ctDeathsAverted", 0, tMax, pLength),
  ctDeathsAvertedHIV("ctDeathsAvertedHIV", 0, tMax, pLength),
  ctDeathsAvertedChildren("ctDeathsAvertedChildren", 0, tMax, pLength),
  
  activeHouseholdContacts("activeHouseholdContacts", 0, tMax, pLength),
  activeHouseholdContactsUnder5("activeHouseholdContactsUnder5", 0, tMax, pLength),
  totalHouseholdContacts("totalHouseholdContacts", 0, tMax, pLength),
  totalHouseholdContactsUnder5("totalHouseholdContactsUnder5", 0, tMax, pLength),

  pyramid("Population pyramid", 0, tMax, pLength, 2, ageBreaks),
  deathPyramid("Death pyramid", 0, tMax, pLength, 2, ageBreaks),
  householdsCount("households", 0, tMax, pLength)
{
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

  tbDeaths.Close();
  tbDeathsHIV.Close();
  tbDeathsUnderFive.Close();

  ctHomeVisits.Close();
  ctScreenings.Close();
  ctScreeningsHIV.Close();
  ctScreeningsChildren.Close();
  ctCasesFound.Close();
  ctCasesFoundHIV.Close();
  ctCasesFoundChildren.Close();
  ctDeathsAverted.Close();
  ctDeathsAvertedHIV.Close();
  ctDeathsAvertedChildren.Close();

  tbInTreatment.Close();
  tbCompletedTreatment.Close();
  tbDroppedTreatment.Close();

  activeHouseholdContacts.Close();
  activeHouseholdContactsUnder5.Close();
  totalHouseholdContacts.Close();
  totalHouseholdContactsUnder5.Close();

  return;
}
