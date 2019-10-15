#include <Exponential.h>
#include <Bernoulli.h>

#include "../../include/TBABM/TB.h"

// Evaluates the risk of infection according to age,
// sex, year, CD4 count, and household TB presence.
// May schedule TB infection.
// 
// If infection is scheduled, 'InfectionRiskEvaluate'
// will not schedule itself again. If infection is
// not scheduled, then the next 'risk_window' is computed
// from the same properties used to model infection
// risk, and 'InfectionRiskEvaluate' is rescheduled.
// 
// When scheduling an infection, the only StrainType
// supported right now is 'Unspecified'.
#define SMALLNUM 0.000000000000001

  bool
TB::InfectionRiskEvaluate_impl(Time t, int risk_window_local, shared_p<TB> l_ptr)
{
  assert(l_ptr);

  // Short-circuit if a new risk_window_id is in place, e.g. if 
  // a RiskReeval event has occurred
  if (!AliveStatus() || risk_window_local != risk_window_id)
    return true;

  // Can't get infected if you're not susceptible or latent
  if (tb_status != TBStatus::Susceptible && \
      tb_status != TBStatus::Latent)
    return true;

  // Can't get infected if you're undergoing treatment
  if (tb_treatment_status == TBTreatmentStatus::Incomplete)
    return true;

  bool init_infection {false};
  bool init_active    {false};

  bool previously_treated {tb_treatment_status == TBTreatmentStatus::Complete};

  long double reduction {1};

  HIVType hiv_cat = GetHIVType(t);

  if (previously_treated) {
    if (hiv_cat == HIVType::Neg)
      reduction = 1-params["TB_risk_reduction"].Sample(rng);
    else if (hiv_cat == HIVType::Good)
      reduction = 1-params["TB_risk_reduction_goodHIV"].Sample(rng);
    else // badHIV
      reduction = 1-params["TB_risk_reduction_badHIV"].Sample(rng);
  }
   
  long double risk_global     {reduction * \
                               GlobalTBPrevalence(t) * \
                               params["TB_risk_global"].Sample(rng)};
  long double risk_household  {reduction * \
                               ContactHouseholdTBPrevalence(tb_status) * \
                               params["TB_risk_household"].Sample(rng)};

  // Time to infection for global and local. If any of these risks are zero,
  // change to a really small number since you can't sample 0-rate exponentials
  long double tti_global     {Exponential(risk_global    > 0 ? risk_global    : SMALLNUM)(rng.mt_)};
  long double tti_household  {Exponential(risk_household > 0 ? risk_household : SMALLNUM)(rng.mt_)};

  // First risk window of the simulation, we infect a bunch of people. Some of them get infected
  // with latent TB, and some of them go straight to active disease. "TB_p_init_active" is the
  // probability that someone who is gets infected initially (vis-as-vis "TB_p_init_infect") will
  // go straight to active disease.
  if (t < risk_window && params["TB_p_init_infect"].Sample(rng))
    init_infection = true;
  if (t < risk_window && params["TB_p_init_active"].Sample(rng) && init_infection)
    init_active = true;

  // If they do get infected, this is what the infection source is. Note: During 'seeding'
  // (0 < t < risk_window) all infection is from the community/global
  Source infection_source {tti_global < tti_household ? Source::Global : Source::Household};
  if (init_infection)
    infection_source = Source::Global;

  long double master_infection_time {init_infection ? 0.0 : std::min(tti_global, tti_household)};

  // Will this individual be infected now?
  if (master_infection_time < risk_window/365) {

    // Will they become latently infected, or progress rapidly?
    // Individuals who are latently infected CANNOT become latently infected
    // again; after this sampling they are only vulnerable to rapid progression
    HIVType hiv_cat = GetHIVType(t);

    Param risk;

    if (hiv_cat == HIVType::Neg)
      risk = params["TB_rapidprog_risk"];
    else if (hiv_cat == HIVType::Good)
      risk = params["TB_rapidprog_risk_goodHIV"];
    else
      risk = params["TB_rapidprog_risk_badHIV"];

    if (risk.Sample(rng) || init_active)
      InfectInfectious(t + 365*master_infection_time, infection_source, StrainType::Unspecified);
    else if (tb_status == TBStatus::Susceptible)
      InfectLatent(t + 365*master_infection_time, infection_source, StrainType::Unspecified);

  } else {
    InfectionRiskEvaluate(t + risk_window, risk_window_local, std::move(l_ptr));
  }

  return true;	
}

  void
TB::InfectionRiskEvaluate(Time t, int risk_window_local, shared_p<TB> l_ptr)
{
  assert(l_ptr);

  auto lambda = [this, risk_window_local, l_ptr] (auto ts, auto) -> bool {
    return InfectionRiskEvaluate_impl(ts, risk_window_local, l_ptr);
  };

  eq.QuickSchedule(t, lambda);
  return;
}

  void
TB::InfectionRiskEvaluate_initial(int local_risk_window)
{
  auto l_ptr = GetLifetimePtr();
  if (!l_ptr) {
    printf("Empty l_ptr in IRE_initial\n");
    return;
  }

  double firstRiskEval = Uniform(0, risk_window)(rng.mt_);
  auto lambda = [this, local_risk_window, l_ptr] (auto ts, auto) -> bool {
    return InfectionRiskEvaluate_impl(ts, local_risk_window, std::move(l_ptr));
  };

  eq.QuickSchedule(init_time + firstRiskEval, lambda);
  return;
}
