#include "../../include/TBABM/TB.h"

void
TB::ContactTrace(Time t)
{
  auto lambda = [this, l_ptr = GetLifetimePtr()] (auto ts, auto) -> bool {
    assert(l_ptr);

    if (!AliveStatus())
      return true;

    if (tb_status == TBStatus::Infectious &&
        tb_treatment_status != TBTreatmentStatus::Incomplete)
      Log(ts, string("Successful contact trace!"));

    return true;
  };

  eq.QuickSchedule(t, lambda);

  return;
}
