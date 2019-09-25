#include "../../include/TBABM/TB.h"

// Right now, changing the risk window is disabled. However, calling this function will
// still trigger an immediate InfectionRiskEvaluate and disable any other scheduled
// InfectionRiskEvaluates.
  void
TB::RiskReeval(Time t)
{
  auto lambda = [this, l_ptr = GetLifetimePtr()] (auto ts, auto) -> bool {
    assert(l_ptr);

    if (!AliveStatus())
      return true;

    // Don't do risk re-evals during the 'seeding' period
    if (ts < risk_window)
      return true;

    auto old_risk_window = risk_window;

    // Log(ts, "TB: RiskReeval triggered");

    // CONDITIONS THAT MAY CHANGE RISK WINDOW
    // if (HIVStatus() == HIVStatus::Positive)
    // risk_window = 2*365; // unit: [days]

    // /END CONDITIONS THAT MAY CHANGE RISK WINDOW

    // If you have active disease, you shouldn't be running
    // 'InfectionRiskEvaluate' in the first place
    if (tb_status != TBStatus::Susceptible && \
        tb_status != TBStatus::Latent)
      return true;

    risk_window_id += 1;

    InfectionRiskEvaluate(ts, risk_window_id, std::move(l_ptr));

    return true;
  };

  eq.QuickSchedule(t, lambda);

  return;
}
