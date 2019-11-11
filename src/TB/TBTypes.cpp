#include "../../include/TBABM/TBTypes.h"

TBData CreateTBData(IndividualInitData data) {
  return {
    data.tbInfections,
      data.tbIncidence,
      data.tbRecoveries,
      data.tbInfectionsHousehold,
      data.tbInfectionsCommunity,
      data.tbSusceptible,
      data.tbLatent,
      data.tbInfectious,
      data.tbExperienced,
      data.tbExperiencedPyr,

      data.tbTreatmentBegin,
      data.tbTreatmentBeginHIV,
      data.tbTreatmentBeginChildren,
      data.tbTreatmentBeginAdultsNaive,
      data.tbTreatmentBeginAdultsExperienced,

      data.tbTreatmentEnd,
      data.tbTreatmentDropout,
      data.tbInTreatment,
      data.tbCompletedTreatment,
      data.tbDroppedTreatment,

      data.tbTxExperiencedAdults,
      data.tbTxExperiencedInfectiousAdults,
      data.tbTxNaiveAdults,
      data.tbTxNaiveInfectiousAdults,

      data.ctHomeVisits,
      data.ctScreenings,
      data.ctCasesFound,
      data.ctDeathsAverted,
      data.ctInfectiousnessAverted,

      data.activeHouseholdContacts
  };
}

TBHandlers CreateTBHandlers(function<void(int)> death) {
  return {
    death
  };
}


TBQueryHandlers CreateTBQueryHandlers(function<int(Time)> Age,
    function<bool(void)> Alive,
    function<double(Time)> CD4Count,
    function<HIVStatus(void)> GetHIVStatus,
    function<bool(void)> ARTStatus,
    function<double(Time)> GlobalTBPrevalence,
    function<shared_p<TB>(void)> Lifetime)
{
  if (!Age || !Alive || \
      !CD4Count || !GetHIVStatus || !ARTStatus || \
      !GlobalTBPrevalence || !Lifetime) {
    printf("Error: >= 1 argument to CreateTBHandlers contained empty std::function\n");
    exit(1);
  }

  return {
    std::move(Age),
    std::move(Alive),
    std::move(CD4Count),
    std::move(GetHIVStatus),
    std::move(ARTStatus),
    std::move(GlobalTBPrevalence),
    std::move(Lifetime)
  };
}
