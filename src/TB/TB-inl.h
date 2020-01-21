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
TB::SetHouseholdCallbacks(
  function<ContactTraceResult(const Time&, Param&, Param&, RNG&)> contactTrace,
  function<void(Time)>       progression, 
  function<void(Time)>       recovery,
  function<double(void)>     householdPrevalence,
  function<double(TBStatus)> contactHouseholdPrevalence)
{
  if (!(contactTrace &&
        progression && 
        recovery && 
        householdPrevalence && 
        contactHouseholdPrevalence)) {

    printf("A household callback in SetHouseholdCallbacks was empty\n");
    exit(1);
  }

  ContactTraceHandler          = contactTrace;
  ProgressionHandler           = progression;
  RecoveryHandler              = recovery;
  HouseholdTBPrevalence        = householdPrevalence;
  ContactHouseholdTBPrevalence = contactHouseholdPrevalence;
}

  void
TB::ResetHouseholdCallbacks(void)
{
  ContactTraceHandler          = nullptr;
  ProgressionHandler           = nullptr;
  RecoveryHandler              = nullptr;
  HouseholdTBPrevalence        = nullptr;
  ContactHouseholdTBPrevalence = nullptr;
}

// This function is an interface to the death mechanism provided
// by Individual. It makes sure that the 'flag_contact_trace' has not been
// put up before scheduling the Individual-level death mechanism. It is only
// run for a TB death.
void
TB::InternalDeathHandler(Time t)
{
  auto lambda = [this, lifetm = GetLifetimePtr()] (auto ts_, auto) -> bool {

    assert(lifetm);

    if (!AliveStatus())
      return true;

    auto ts = static_cast<int>(ts_);

    // If this event has been flagged because of a contact-trace, remove the flag
    // and pretend the event never happened.
    if (flag_contact_traced) {
      assert(flag_date != -1);

      data.ctDeathsAverted.Record(ts, +1);

      if (GetHIVStatus() == HIVStatus::Positive)
        data.ctDeathsAvertedHIV.Record(ts, +1);
      if (AgeStatus(ts) < 5)
        data.ctDeathsAvertedChildren.Record(ts, +1);
      
      data.ctInfectiousnessAverted(ts - flag_date);
      
      flag_contact_traced = false;
      flag_date = -1;

      return true;
    }
    
    data.tbDeaths.Record(ts, +1);
    if (GetHIVStatus() == HIVStatus::Positive)
      data.tbDeathsHIV.Record(ts, +1);
    if (AgeStatus(ts) < 5)
      data.tbDeathsUnderFive.Record(ts, +1);

    // Otherwise, schedule the death for time 't'.
    DeathHandler(ts);

    return true;
  };

  eq.QuickSchedule(t, lambda);
}

// This function is called by Individual AFTER death is assured to happen.
// The above function is used to check the contact-tracing flag BEFORE a 
// death is assured to happen.
void
TB::HandleDeath(Time t)
{
  bool adult = AgeStatus(t) >= 15;

  auto ts = static_cast<int>(t);

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
