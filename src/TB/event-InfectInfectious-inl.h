#include "../../include/TBABM/TB.h"

// Marks an individual as infected and may or may not
// schedule the beginning of treatment.
// 
// If no treatment, recovery or death is scheduled.
void
TB::InfectInfectious(Time t, Source s, StrainType)
{
  auto lambda = [this, s, lifetm = GetLifetimePtr()] (auto ts_, auto) {
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
    risk_window_id += 1;

    if (tb_status == TBStatus::Latent)
      data.tbLatent.Record(ts, -1);
    if (tb_status == TBStatus::Susceptible)
      data.tbSusceptible.Record(ts, -1);

    data.tbInfectious.Record(ts, +1);
    data.tbIncidence.Record(ts, +1);

    if (s == Source::Global)
      data.tbInfectionsCommunity.Record(ts, +1);
    else if (s == Source::Household)
      data.tbInfectionsHousehold.Record(ts, +1);


    if (treatment_experienced && \
        AgeStatus(ts) >= 15)
      data.tbTxExperiencedInfectiousAdults.Record(ts, +1);

    if (!treatment_experienced && \
        AgeStatus(ts) >= 15)
      data.tbTxNaiveInfectiousAdults.Record(ts, +1);

    // Mark as infectious
    tb_status = TBStatus::Infectious;

    assert(ProgressionHandler);
    ProgressionHandler(ts);

    Param t_seek_tx;
    Param t_death;
    Param t_recov;

    HIVType hiv_cat = GetHIVType(ts);

    long double seek_tx_base_rate { 1.0/params["TB_seek_tx_base_time"].Sample(rng) };
    
    if (treatment_experienced) {
      if (hiv_cat == HIVType::Neg) {
        seek_tx_base_rate  *= params["TB_seek_tx_pt_scalar"].Sample(rng);
        t_death             = params["TB_t_death"];
        t_recov             = params["TB_t_recov"];
      } else if (hiv_cat == HIVType::Good) {
        seek_tx_base_rate  *= params["TB_seek_tx_pt_goodHIV_scalar"].Sample(rng);
        t_death             = params["TB_t_death_goodHIV"];
        t_recov             = params["TB_t_recov_goodHIV"];
      } else {
        seek_tx_base_rate  *= params["TB_seek_tx_pt_badHIV_scalar"].Sample(rng);
        t_death             = params["TB_t_death_badHIV"];
        t_recov             = params["TB_t_recov_badHIV"];
      }
    } else {
      if (hiv_cat == HIVType::Neg) {
        seek_tx_base_rate  *= 1;
        t_death             = params["TB_t_death"];
        t_recov             = params["TB_t_recov"];
      } else if (hiv_cat == HIVType::Good) {
        seek_tx_base_rate  *= params["TB_seek_tx_goodHIV_scalar"].Sample(rng);
        t_death             = params["TB_t_death_goodHIV"];
        t_recov             = params["TB_t_recov_goodHIV"];
      } else {
        seek_tx_base_rate  *= params["TB_seek_tx_badHIV_scalar"].Sample(rng);
        t_death             = params["TB_t_death_badHIV"];
        t_recov             = params["TB_t_recov_badHIV"];
      }
    }

    // We have now computed the rate of seeking treatment for this individual.
    auto seek_tx_rate = seek_tx_base_rate;

    auto timeToNaturalRecovery	= t_recov.Sample(rng);
    auto timeToDeath            = t_death.Sample(rng);
    auto timeToSeekingTreatment = Exponential(seek_tx_rate)(rng.mt_);

    auto winner =
      std::min({timeToNaturalRecovery,
          timeToDeath,
          timeToSeekingTreatment});

    // Individual recovers naturally
    if (winner == timeToNaturalRecovery)
      Recovery(ts + 365*timeToNaturalRecovery, RecoveryType::Natural);

    // Individual dies from infection
    else if (winner == timeToDeath)
      InternalDeathHandler(ts + 365*timeToDeath);

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
