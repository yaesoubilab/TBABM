#include "../../include/TBABM/TB.h"

// Marks an individual as latently infected. Records information
// to relevent data sources, and calls MaintainLatent(3)
  void
TB::InfectLatent(Time t, Source source, StrainType strain)
{
  auto lambda = [this, 
       source, 
       strain, 
       lifetm = GetLifetimePtr()] (auto ts_, auto) {

         assert(lifetm);

         auto ts = static_cast<int>(ts_);

         if (!AliveStatus())
           return true;

         // Log(ts, "TB infection: Latent");
         if (tb_status == TBStatus::Infectious)
           return true;

         // If they have no history of latent TB infection
         // NOTE: May change if TB history items become more robust!
         // If this is a new infection, add it to the individual's
         // history
         if (tb_status == TBStatus::Susceptible) {
           data.tbSusceptible.Record(ts, -1);
           data.tbInfections.Record(ts, +1);
           data.tbExperienced.Record(ts, +1);
           tb_history.emplace_back(ts, source, strain);
         }

         if (tb_status != TBStatus::Latent)
           data.tbLatent.Record(ts, +1);

         // Mark as latently infected
         tb_status = TBStatus::Latent;

         // InfectLatent calls
         if (source == Source::Global)
           data.tbInfectionsCommunity.Record(ts, +1);
         else if (source == Source::Household)
           data.tbInfectionsHousehold.Record(ts, +1);

         // NOTE: right now, you always have the same source and strain
         // as your first TB infection!
         long double timeToActiveDisease = 365*params["TB_prog_time"].Sample(rng);

         InfectInfectious(ts + timeToActiveDisease, source, strain);

         return true;
       };

  eq.QuickSchedule(t, lambda);

  return;
}
