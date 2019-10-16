#include "../../include/TBABM/TB.h"

int
TB::ContactTrace(Time t)
{
  if (!AliveStatus())
    return true;

  if (tb_status == TBStatus::Infectious &&
      tb_treatment_status != TBTreatmentStatus::Incomplete)
    return 1;

  return 0;
}
