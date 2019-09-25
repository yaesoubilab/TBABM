#include "../../include/TBABM/TB.h"

  void
TB::TreatmentComplete(Time t)
{
  auto lambda = [this, lifetm = GetLifetimePtr()] (auto ts_, auto) -> bool {
    assert(lifetm);

    auto ts = static_cast<int>(ts_);

    if (!AliveStatus())
      return true;

    // Log(ts, "TB treatment complete");

    data.tbInTreatment.Record(ts, -1);
    data.tbTreatmentEnd.Record(ts, +1);
    data.tbCompletedTreatment.Record(ts, +1);

    tb_treatment_status = TBTreatmentStatus::Complete;

    Recovery(ts, RecoveryType::Treatment);

    return true;
  };

  eq.QuickSchedule(t, lambda);

  return;
}
