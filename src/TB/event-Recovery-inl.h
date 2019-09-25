#include "../../include/TBABM/TB.h"

  void
TB::Recovery(Time t, RecoveryType r)
{
  auto lambda = [this, r, l_ptr = GetLifetimePtr()] (auto ts, auto) -> bool {
    assert(l_ptr);

    if (!AliveStatus())
      return true;

    // Log(ts, string("TB recovery: ") + (r == RecoveryType::Natural ? "natural" : "treatment"));

    data.tbRecoveries.Record((int)ts, +1);

    if (AgeStatus(ts) >= 15 && r != RecoveryType::Treatment) {
      if (!treatment_experienced)
        data.tbTxNaiveInfectiousAdults.Record((int)ts, -1);
      else
        data.tbTxExperiencedInfectiousAdults.Record((int)ts, -1);
    }

    data.tbInfectious.Record((int)ts, -1);
    data.tbLatent.Record((int)ts, +1);

    tb_status = TBStatus::Latent;

    // If they recovered and it's not because they achieved treatment
    // completion, call the RecoveryHandler
    if (RecoveryHandler && r == RecoveryType::Natural)
      RecoveryHandler(ts);

    // Set up periodic evaluation for reinfection
    InfectionRiskEvaluate(ts, risk_window, std::move(l_ptr));

    // Set up one-time sample for reactivation
    InfectLatent(ts, 
        Source::Global, 
        StrainType::Unspecified);

    return true;
  };

  eq.QuickSchedule(t, lambda);

  return;
}
