#include "../../include/TBABM/TB.h"

// Marks an individual as infected and may or may not
// schedule the beginning of treatment.
// 
// If no treatment, recovery or death is scheduled.
void
TB::InfectInfectious(Time t, Source s, StrainType)
{
  auto lambda = [this, lifetm = GetLifetimePtr()] (auto ts_, auto) {
    auto ts = static_cast<int>(ts_);

    assert(lifetm);

    if (!AliveStatus())
      return true;

    // You can't become infectious if you're already infectious
    if (tb_status == TBStatus::Infectious)
      return true;

    // Log(ts, "TB infection: Infectious");

    // Prevent InfectionRiskEvaluate from continuing to run
    // (this could happen in the case that an individual is
    // re-activating right now - it shouldn't be possible for
    // them to develop a re-infection)
    risk_window += 1;

    if (tb_status == TBStatus::Latent)
      data.tbLatent.Record(ts, -1);
    if (tb_status == TBStatus::Susceptible)
      data.tbSusceptible.Record(ts, -1);

    data.tbInfectious.Record(ts, +1);
    data.tbIncidence.Record(ts, +1);

    if (treatment_experienced && \
        AgeStatus(ts) >= 15)
      data.tbTxExperiencedInfectiousAdults.Record(ts, +1);

    if (!treatment_experienced && \
        AgeStatus(ts) >= 15)
      data.tbTxNaiveInfectiousAdults.Record(ts, +1);

    // Mark as infectious
    tb_status = TBStatus::Infectious;

    if (ProgressionHandler)
      ProgressionHandler(ts);

    Param t_seek_tx;
    HIVType hiv_cat = GetHIVType(ts);
    
    if (treatment_experienced) {
      if (hiv_cat == HIVType::Neg)
        t_seek_tx = params["TB_t_seek_tx_pt"];
      else if (hiv_cat == HIVType::Good)
        t_seek_tx = params["TB_t_seek_tx_pt_goodHIV"];
      else
        t_seek_tx = params["TB_t_seek_tx_pt_badHIV"];
    } else {
      if (hiv_cat == HIVType::Neg)
        t_seek_tx = params["TB_t_seek_tx"];
      else if (hiv_cat == HIVType::Good)
        t_seek_tx = params["TB_t_seek_tx_goodHIV"];
      else
        t_seek_tx = params["TB_t_seek_tx_badHIV"];
    }

    auto timeToNaturalRecovery	= params["TB_t_recov"].Sample(rng);
    auto timeToDeath            = params["TB_t_death"].Sample(rng);
    auto timeToSeekingTreatment = t_seek_tx.Sample(rng);

    auto winner =
      std::min({timeToNaturalRecovery,
          timeToDeath,
          timeToSeekingTreatment});

    // Individual recovers naturally
    if (winner == timeToNaturalRecovery)
      Recovery(ts + 365*timeToNaturalRecovery, RecoveryType::Natural);

    // Individual dies from infection
    else if (winner == timeToDeath)
      DeathHandler(ts + 365*timeToDeath);

    // Individual seeks out treatment
    else if (winner == timeToSeekingTreatment)
      TreatmentBegin(ts + 365*timeToSeekingTreatment);

    // Something bad happened
    else {
      printf("Unsupported progression from Infectious state!\n");
      exit(1);
    }

    return true;
  };

  eq.QuickSchedule(t, lambda);

  return;
}
