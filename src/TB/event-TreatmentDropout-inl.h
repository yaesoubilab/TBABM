#include "../../include/TBABM/TB.h"

  void
TB::TreatmentDropout(Time t)
{
  auto lambda = [this, lifetm = GetLifetimePtr()] (auto ts_, auto) -> bool {
    assert(lifetm);

    auto ts = static_cast<int>(ts_);

    if (!AliveStatus())
      return true;

    // Log(ts, "TB treatment dropout");

    data.tbInTreatment.Record(ts, -1);
    data.tbDroppedTreatment.Record(ts, +1); 
    data.tbTreatmentDropout.Record(ts, +1);

    tb_treatment_status = TBTreatmentStatus::Dropout;

    return true;
  };

  eq.QuickSchedule(t, lambda);

  return;
}
