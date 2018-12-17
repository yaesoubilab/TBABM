#include <functional>
#include <map>
#include <string>

#include <Param.h>
#include <DataFrame.h>
#include <IncidenceTimeSeries.h>

using std::function;
using namespace SimulationLib;

template <typename T>
using Pointer = std::shared_ptr<T>;

enum class StrainType {Unspecified};
enum class RecoveryType {Natural, Treatment};
using TBStatus = TBStatus;

using Params = std::map<std::string, Param>;

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

	TB(void) {return;}

	TB(SexT sex,
	   function<Age(Time)> AgeStatus,
	   function<Alive(void)> AliveStatus,
	   function<CD4(Time)> CD4Count,
	   function<HouseholdTB(void)> HouseholdStatus,

	   function<void(Time)> DeathHandler,
	   
	   double default_risk_window, // unit: [days]
	   
	   // Pointer<IncidenceTimeSeries<int>> TBIncidence,

	   // Pointer<Params> params,
	   // Pointer<map<string, DataFrameFile>> fileData,

	   TBStatus tb_status = TBStatus::Susceptible) :
		sex(sex),
		AgeStatus(AgeStatus),
		CD4Count(CD4Count),
		HouseholdStatus(HouseholdStatus),
		DeathHandler(DeathHandler),
		default_risk_window(default_risk_window),
		// TBIncidence(TBIncidence),
		// params(params)
		// fileData(fileData),
		tb_status(tb_status) {}

	~TB(void);

	TBStatus GetTBStatus(void);
	void RiskReeval(void);
	void Investigate(void);

private:

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
	void InfectionRiskEvaluate(Time);

	// Marks an individual as infected and may or may not
	// schedule the beginning of treatment.
	// 
	// If no treatment, recovery or death is scheduled.
	void Infect(Time, StrainType);

	// Marks an individual as having begun treatment.
	// Decides if they will complete treatment, or drop
	// out. Schedules either event.
	void TreatmentBegin(Time);


	void TreatmentDropout(Time);
	void TreatmentComplete(Time);


	void Recovery(Time, RecoveryType);

	double risk_window; // unit: [days]

	TBStatus tb_status;


	// All of these are from the constructor
	SexT sex;
    function<Age(Time)> AgeStatus;
    function<Alive(void)> AliveStatus;
    function<CD4(Time)> CD4Count;
    function<HouseholdTB(void)> HouseholdStatus;
 
    function<void(Time)> DeathHandler;
    
    double default_risk_window;
    
    // Pointer<IncidenceTimeSeries<int>> TBIncidence;
 
    // Pointer<Params> params;
    // Pointer<map<string, DataFrameFile>> fileData;
};