#include <functional>
#include <map>
#include <string>

#include <RNG.h>
#include <Uniform.h>
#include <EventQueue.h>
#include <Param.h>
#include <DataFrame.h>
#include <IncidenceTimeSeries.h>

#include "IndividualTypes.h"
#include "TBTypes.h"

using std::function;
using std::string;

using namespace SimulationLib;

template <typename T>
using Pointer = std::shared_ptr<T>;

using Params = std::map<std::string, Param>;

using EQ = EventQueue<double, bool>;

template <typename SexT>
class TB
{
public:
	using Time        = double;
	using Alive       = bool;
	using Age         = int;
	using Year        = int;
	using CD4         = double;
	using HouseholdTB = bool;

	TB(TBData initData,
	   TBSimContext initCtx,
	   TBHandlers initHandlers,
	   string name,
	   SexT sex,
	   function<Age(Time)> AgeStatus,
	   function<Alive(void)> AliveStatus,
	   function<CD4(Time)> CD4Count,
	   function<HIVStatus(void)> HIVStatus,
	   function<double(Time)> GlobalTBPrevalence,
	   function<double(Time)> HouseholdTBPrevalence,

	   double risk_window, // unit: [days]
	   
	   TBStatus tb_status = TBStatus::Susceptible) :
		name(name),
		sex(sex),
		AgeStatus(AgeStatus),
		AliveStatus(AliveStatus),
		CD4Count(CD4Count),
		HIVStatus(HIVStatus),
		GlobalTBPrevalence(GlobalTBPrevalence),
		HouseholdTBPrevalence(HouseholdTBPrevalence),

		DeathHandler(initHandlers.death),

		data(initData),
		eq(initCtx.event_queue),
		rng(initCtx.rng),
		fileData(initCtx.fileData),
		params(initCtx.params),

		risk_window(risk_window),
		tb_status(tb_status),
		tb_treatment_status(TBTreatmentStatus::None),
		risk_window_id(0)
	{
		double firstRiskEval = Uniform(0, risk_window)(rng.mt_);

		InfectionRiskEvaluate(initCtx.current_time + firstRiskEval);

		data.tbSusceptible.Record(initCtx.current_time, +1);
	}

	~TB(void);

	TBStatus GetTBStatus(Time);

	void RiskReeval(Time);
	void Investigate(void);

	// Called by containing Individual upon death, neccessary
	// to update TimeSeries data.
	void HandleDeath(Time);

private:

	void Log(Time, string);

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
	void InfectionRiskEvaluate(Time, int local_risk_window = 0);

	// Marks an individual as latently infected. May transition
	// to infectous TB through reactivation.
	// 
	// NOTE: Reinfection not implemented
	void InfectLatent(Time, StrainType);

	// Marks an individual as infectous and may or may not
	// schedule the beginning of treatment.
	// 
	// If no treatment, recovery or death is scheduled.
	void InfectInfectious(Time, StrainType);

	// Marks an individual as having begun treatment.
	// Decides if they will complete treatment, or drop
	// out. Schedules either event.
	void TreatmentBegin(Time);

	// Sets tb_treatment_status to Incomplete
	void TreatmentDropout(Time);

	// Sets tb_treatment_status to Complete, and 
	// calls Recovery
	void TreatmentComplete(Time);

	// Marks the individual as recovered, and sets
	// status tb_status to Latent
	void Recovery(Time, RecoveryType);

	double risk_window; // unit: [days]
	int risk_window_id;

	TBStatus tb_status;
	TBTreatmentStatus tb_treatment_status;

	// All of these are from the constructor
	string name;
	SexT sex;

	EQ& eq;
	RNG& rng;
	map<string, DataFrameFile>& fileData;
	Params& params;

	TBData data;

    function<Age(Time)> AgeStatus;
    function<Alive(void)> AliveStatus;
    function<CD4(Time)> CD4Count;
    function<HIVStatus(void)> HIVStatus;
    function<double(Time)> GlobalTBPrevalence;
	function<double(Time)> HouseholdTBPrevalence;
 
    function<void(Time)> DeathHandler;        
};