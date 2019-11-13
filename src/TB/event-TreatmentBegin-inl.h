#include "../../include/TBABM/TB.h"

// Marks an individual as having begun treatment.
// Decides if they will complete treatment, or drop
// out. Schedules either event.
// For the purposes of surveillance, etc.. this is 
// also considered diagnosis right now
//
// 'flag_override' prevents a TreatmentBegin event from being cancelled in
// the event that the ORIGIN of the event was the contact-trace itself.
  void
TB::TreatmentBegin(Time t, bool flag_override)
{
  auto lambda = [this, flag_override, lifetm = GetLifetimePtr()]
                (auto ts_, auto) -> bool {

    assert(lifetm);
    assert(tb_treatment_status != TBTreatmentStatus::Incomplete);

    auto ts = static_cast<int>(ts_);

    if (!AliveStatus())
      return true;

    // Don't begin treatment it was begun in the past becasue of a contact
    // trace.
    if (flag_contact_traced && !flag_override) {
     
      data.ctInfectiousnessAverted(ts - flag_date);

      flag_date = -1; // Reset the flag_date
      flag_contact_traced = false;

      return true;
    }

    if (flag_contact_traced && flag_override) {
      // No-op
    }

    if (tb_status != TBStatus::Infectious) {
      printf("warn: Can't begin tx for non-infectious TB. Flag was (%d), override was (%d)\n",
             (int)flag_contact_traced,
             (int)flag_override);
      return true;
    }

    // Log(ts, "TB treatment begin");

    auto prev_household = ContactHouseholdTBPrevalence(tb_status);

    data.tbInTreatment.Record(ts, +1);
    data.tbTreatmentBegin.Record(ts, +1);

    // Subgroups of treatmentBegin for children, treatment-naive adults,
    // treatment-experienced adults
    if (AgeStatus(ts) < 15)
      data.tbTreatmentBeginChildren.Record(ts, +1);
    else if (tb_treatment_status == TBTreatmentStatus::None)
      data.tbTreatmentBeginAdultsNaive.Record(ts, +1);
    else
      data.tbTreatmentBeginAdultsExperienced.Record(ts, +1);

    // Subgroup for HIV+ people
    if (GetHIVStatus() == HIVStatus::Positive)
      data.tbTreatmentBeginHIV.Record(ts, +1);

    if (tb_treatment_status == TBTreatmentStatus::Dropout)
      data.tbDroppedTreatment.Record(ts, -1);
    else if (tb_treatment_status == TBTreatmentStatus::Complete)
      data.tbCompletedTreatment.Record(ts, -1);

    data.activeHouseholdContacts.Record(prev_household);

    tb_treatment_status = TBTreatmentStatus::Incomplete;

    if (RecoveryHandler)
      RecoveryHandler(ts);

    // Will they complete treatment? Assume 100% yes
    if (params["TB_p_Tx_cmp"].Sample(rng))
      TreatmentComplete(ts + 365*params["TB_t_Tx_cmp"].Sample(rng),
                        flag_override);
    else
      TreatmentDropout(ts + 365*params["TB_t_Tx_drop"].Sample(rng));

    // Schedule the moment where they will be marked as "treatment-experienced."
    // Right now, this is 1 month after treatment start
    TreatmentMarkExperienced(ts + 1*30, flag_override);

    // Don't do contact tracing until 20 years. Also, don't do it if this
    // TreatmentBegin event is the result of a contact trace. This makes sense
    // in the case where contact tracing is limited to the household.
    if (ts > 365*20 && // At least 2010
        !flag_override && // This isn't part of an existing contact trace
        params["TB_CT_frac_visit"].Sample(rng)) { // We're actually going
                                                  // to visit the HH.
      
      auto delay = 365*params["TB_CT_t_visit"].Sample(rng);

      // Schedule a contact trace
      // Remember: have to capture the ContactTraceHandler because individual
      // might die, but we still want to CT his/her household
      eq.QuickSchedule(ts + delay, 
        [this, lifetm, cth=ContactTraceHandler] (auto ts_, auto) -> bool {

        // Do NOT trace household if dead!! Memory errors...could develop a
        // workaround to this problem. Issue is that when the dead person is
        // the last person in their household to die, the household is deleted,
        // so there would have to be a way to detect that the household doesn't
        // exist without actually accessing it and segfaulting.
        if (!AliveStatus())
          return true;

        auto result = 
          cth(ts_, params["TB_CT_frac_screened"], rng);

        data.ctHomeVisits.Record(ts_, +1);

        data.ctCasesFound.Record(ts_,         result.cases_found);
        data.ctCasesFoundHIV.Record(ts_,      result.cases_found_hiv);
        data.ctCasesFoundChildren.Record(ts_, result.cases_found_children);

        data.ctScreenings.Record(ts_,         result.screenings);
        data.ctScreeningsHIV.Record(ts_,      result.screenings_hiv);
        data.ctScreeningsChildren.Record(ts_, result.screenings_children);

        return true;
      });

    } else if (ts > 365*20 && !flag_override) { // Not going to trace
      // printf("Not going to trace\n");
    }

    return true;
  };

  eq.QuickSchedule(t, lambda);

  return;
}

  void
TB::TreatmentMarkExperienced(Time t, bool flag_override)
{
  auto lambda = [this, flag_override, lifetm = GetLifetimePtr()]
                (auto ts_, auto) -> bool {

    assert(lifetm);

    if (!AliveStatus())
      return true;

    if (flag_contact_traced && !flag_override) {
      flag_contact_traced = false;
      // printf("ctrace-cancel-markex,1\n");
      return true;
    }

    if (tb_treatment_status != TBTreatmentStatus::Incomplete)
      return true;

    if (!treatment_experienced && AgeStatus(ts_) >= 15) {
      data.tbTxNaiveAdults.Record((int)ts_, -1);
      data.tbTxNaiveInfectiousAdults.Record((int)ts_, -1);
      data.tbTxExperiencedAdults.Record((int)ts_, +1);

      // Because they aren't considered infectious anymore
      data.tbTxExperiencedInfectiousAdults.Record((int)ts_, 0);
    }

    if (treatment_experienced && AgeStatus(ts_) >= 15)
      data.tbTxExperiencedInfectiousAdults.Record((int)ts_, -1);

    treatment_experienced = true;

    return true;
  };

  eq.QuickSchedule(t, lambda);

  return;
}
