#include "../../include/TBABM/TB.h"

void
TB::TreatmentComplete(Time t)
{
	auto lambda = [this, lifetm = GetLifetimePtr()] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		// Log(ts, "TB treatment complete");

		data.tbTreatmentEnd.Record((int)ts, +1);
		data.tbCompletedTreatment.Record((int)ts, +1);

		tb_treatment_status = TBTreatmentStatus::Complete;

		// Recovery(ts, RecoveryType::Treatment);
		
		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}