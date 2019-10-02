#include <iostream>

#include "../../include/TBABM/TB.h"
#include "../../include/TBABM/utils/termcolor.h"

  void
TB::Log(Time t, string msg)
{
  std::cout << termcolor::on_green << "[" << std::left \
    << std::setw(12) << name << std::setw(5) << std::right \
    << (int)t << "] " \
    << msg << termcolor::reset << std::endl;
}

  TBStatus
TB::GetTBStatus(Time t)
{
  return tb_status;
}

  TB::HIVType
TB::GetHIVType(Time t)
{
  if (GetHIVStatus() == HIVStatus::Negative)
    return HIVType::Neg;
  else if (ARTStatus() || CD4Count(t) > 100)
    return HIVType::Good;
  else
    return HIVType::Bad;
}

  void
TB::SetHouseholdCallbacks(function<void(Time)>       progression, 
    function<void(Time)>       recovery,
    function<double(void)>     householdPrevalence,
    function<double(TBStatus)> contactHouseholdPrevalence)
{
  assert(progression && 
      recovery && 
      householdPrevalence && 
      contactHouseholdPrevalence);

  ProgressionHandler           = progression;
  RecoveryHandler              = recovery;
  HouseholdTBPrevalence        = householdPrevalence;
  ContactHouseholdTBPrevalence = contactHouseholdPrevalence;
}

  void
TB::ResetHouseholdCallbacks(void)
{
  ProgressionHandler    = nullptr;
  RecoveryHandler       = nullptr;
  HouseholdTBPrevalence = nullptr;
}

  void
TB::HandleDeath(Time t)
{
  bool adult = AgeStatus(t) >= 15;

  switch(tb_status) {
    case (TBStatus::Susceptible):
      data.tbSusceptible.Record(t, -1); break;

    case (TBStatus::Latent):
      data.tbLatent.Record(t, -1); 
      data.tbExperienced.Record(t, -1); break;

    case (TBStatus::Infectious):
      // Remember, while people are in treatment, they are 
      // still considered infectious by the value of 'tb_status',
      // but are not functionally infectious
      if (tb_treatment_status != TBTreatmentStatus::Incomplete)
        data.tbInfectious.Record(t, -1);

      data.tbExperienced.Record(t, -1); break;

      break;

    default: std::cout << "Error: UNSUPPORTED TBStatus!" << std::endl;
  }

  switch(tb_treatment_status) {
    case (TBTreatmentStatus::None):
      break;

    case (TBTreatmentStatus::Incomplete):
      data.tbInTreatment.Record(t, -1); break;

    case (TBTreatmentStatus::Complete):
      data.tbCompletedTreatment.Record(t, -1); break;

    case (TBTreatmentStatus::Dropout):
      data.tbDroppedTreatment.Record(t, -1); break;

    default: std::cout << "Error: UNSUPPORTED TBTreatmentStatus!" << std::endl;
  }

  if (treatment_experienced && adult)
    data.tbTxExperiencedAdults.Record(t, -1);
  if (!treatment_experienced && adult)
    data.tbTxNaiveAdults.Record(t, -1);

  if (tb_status == TBStatus::Infectious && \
      !treatment_experienced && \
      adult)
    data.tbTxNaiveInfectiousAdults.Record(t, -1);

  if (tb_status == TBStatus::Infectious && \
      treatment_experienced && \
      adult)
    data.tbTxExperiencedInfectiousAdults.Record(t, -1);

  return;
}

  void
TB::InitialEvents(void)
{
  InfectionRiskEvaluate_initial();
  EnterAdulthood();

  if (AgeStatus(init_time) >= 15 && \
      tb_treatment_status == TBTreatmentStatus::None)
    data.tbTxNaiveAdults.Record(init_time, +1);

  return;
}

void TB::EnterAdulthood(void)
{
  int age_in_years = AgeStatus(init_time);
  int age_of_adulthood = 15;

  if (AgeStatus(init_time) >= 15)
    return;

  int t_enters_adulthood = init_time + 365*(age_of_adulthood-age_in_years);

  auto lambda = [this, 
       t_enters_adulthood, 
       lifetm = GetLifetimePtr()] (auto ts_, auto) -> bool {

         assert(lifetm);

         if (!AliveStatus())
           return true;

         auto ts = static_cast<int>(ts_);

         if (treatment_experienced)
           data.tbTxExperiencedAdults.Record(ts, +1);
         else
           data.tbTxNaiveAdults.Record(ts, +1);

         if (tb_status           == TBStatus::Infectious && \
             treatment_experienced) 
           data.tbTxExperiencedInfectiousAdults.Record(ts, +1);

         if (tb_status           == TBStatus::Infectious && \
             !treatment_experienced) 
           data.tbTxNaiveInfectiousAdults.Record(ts, +1);

         return true;
       };

  eq.QuickSchedule(t_enters_adulthood, lambda);

  return;
}
