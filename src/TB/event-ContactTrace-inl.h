#include "../../include/TBABM/TB.h"

int
TB::ContactTrace(Time t)
{
  if (!AliveStatus())
    return 0;
  
  if (tb_status != TBStatus::Infectious ||
      tb_treatment_status == TBTreatmentStatus::Incomplete)
    return 0;

  flag_contact_traced = true;

  TreatmentBegin(t); // Individual immediately begins treatment

  return 1;
}
