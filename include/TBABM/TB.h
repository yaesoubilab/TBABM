#pragma once

#include <functional>
#include <map>
#include <string>

#include <RNG.h>
#include <Uniform.h>
#include <EventQueue.h>
#include <Param.h>
#include <DataFrame.h>
#include <IncidenceTimeSeries.h>

#include "MasterData.h"
#include "IndividualTypes.h"
#include "HouseholdTypes.h"
#include "TBTypes.h"

using namespace SimulationLib;

using std::function;
using std::string;
using Params = std::map<std::string, Param>;
using EQ = EventQueue<double, bool>;

class TB
{
  public:
    using Time        = double;
    using Alive       = bool;
    using Age         = int;
    using Year        = int;
    using CD4         = double;
    using HouseholdTB = bool;
    using HIVType     = enum class HIVType { Neg, Good, Bad };

    TB(MasterData& initData,
        TBSimContext initCtx,
        TBHandlers initHandlers,
        TBQueryHandlers initQueryHandlers,

        string name,
        Sex sex,

        double risk_window = 3*30, // unit: [days]

        TBStatus tb_status = TBStatus::Susceptible) :
      AgeStatus(initQueryHandlers.Age),
      AliveStatus(initQueryHandlers.Alive),
      CD4Count(initQueryHandlers.CD4Count),
      GetHIVStatus(initQueryHandlers.GetHIVStatus),
      ARTStatus(initQueryHandlers.ART),
      GlobalTBPrevalence(initQueryHandlers.GlobalTBPrevalence),

      name(name),
      sex(sex),

      GetLifetimePtr(initQueryHandlers.Lifetime),

      DeathHandler(initHandlers.death),

      data(initData),
      eq(initCtx.event_queue),
      rng(initCtx.rng),
      fileData(initCtx.fileData),
      params(initCtx.params),

      risk_window(risk_window),
      tb_status(tb_status),
      tb_treatment_status(TBTreatmentStatus::None),
      tb_history({}),
      risk_window_id(0),
      init_time(initCtx.current_time)
      {
        data.tbSusceptible.Record(initCtx.current_time, +1);
      }

    TBStatus GetTBStatus(Time);
    bool PreviouslyTreated(void);

    void SetHouseholdCallbacks(
        function<ContactTraceResult(const Time&, Param&, Param&, RNG&)> contactTrace,
        function<void(Time)> progression, 
        function<void(Time)> recovery,
        function<double(void)> householdPrevalence,
        function<double(TBStatus)> contactHouseholdPrevalence);
    void ResetHouseholdCallbacks(void);

    void InitialEvents(void);

    void RiskReeval(Time);

    // Contact trace: Look at each member of the household. If anyone is
    // infectious, then start them on treatment, and "cancel" any of the
    // following events that may have been scheduled: Death, Recovery,
    // TreatmentBegin
    int ContactTrace(Time);

    // Called by containing Individual upon death, neccessary
    // to update TimeSeries data.
    void HandleDeath(Time);

  private:

    void Log(Time, string);

    //////////////////////////////////////////////////////////////////////////
    // Event functions
    //////////////////////////////////////////////////////////////////////////

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
    void InfectionRiskEvaluate(Time, int local_risk_window = 0, shared_p<TB> = {});
    void InfectionRiskEvaluate_initial(int local_risk_window = 0);
    bool InfectionRiskEvaluate_impl(Time, int local_risk_window = 0, shared_p<TB> = {});

    // Marks an individual as latently infected. May transition
    // to infectous TB through reactivation.
    void InfectLatent(Time, Source, StrainType);

    // Marks an individual as infectous and may or may not
    // schedule the beginning of treatment.
    // 
    // If no treatment, recovery or death is scheduled.
    void InfectInfectious(Time, Source, StrainType);

    // Marks an individual as having begun treatment.
    // Decides if they will complete treatment, or drop
    // out. Schedules either event. The second parameter is the contact-trace
    // flag, which should only be set by the ContactTrace function.
    // The default, 'false' is correct behaviour in the case that the
    // treatment is being initiated by 'InfectInfectious'
    using FastTraceT = enum class FastTraceT { Lead, Lag, None };
    void TreatmentBegin(Time,
                        const bool flag_override = false,
                        const FastTraceT flag_fasttrace = FastTraceT::None);

    void TreatmentMarkExperienced(Time, bool flag_override = false);

    // Sets tb_treatment_status to Incomplete
    void TreatmentDropout(Time);

    // Sets tb_treatment_status to Complete, and 
    // calls Recovery
    void TreatmentComplete(Time, bool flag_override = false);

    // Marks the individual as recovered, and sets
    // status tb_status to Latent
    void Recovery(Time, RecoveryType, bool flag_override = false);

    void InternalDeathHandler(Time);

    //////////////////////////////////////////////////////////////////////////
    // Helper functions
    //////////////////////////////////////////////////////////////////////////

    void EnterAdulthood(void);

    HIVType GetHIVType(Time t);

    //////////////////////////////////////////////////////////////////////////
    // Private member variables
    //////////////////////////////////////////////////////////////////////////

    // Used to invalidate any queued events scheduled by InfectInfectious in
    // the event that the individual's active TB is identified during a
    // contact trace.
    bool flag_contact_traced = false;
    int flag_date = -1; // When the last flag went up
                        // (when the last individual began Tx after being id'd
                        // by a contact-trace intervention

    const double risk_window; // How many days in between evals for LTB. unit: [days]
    int risk_window_id; // The "ID" of the window. Incremented on change in
                        // household prevalence.

    TBStatus tb_status;
    TBTreatmentStatus tb_treatment_status;

    std::vector<TBHistoryItem> tb_history;
    bool treatment_experienced = false;

    // All of these are from the constructor
    string name;
    Sex sex;
    EQ& eq;
    RNG& rng;
    map<string, DataFrameFile>& fileData;
    Params& params;

    int init_time;

    MasterData& data; // Where all the references to timeseries data live

    //////////////////////////////////////////////////////////////////////////
    // Query functions
    //////////////////////////////////////////////////////////////////////////

    function<Age(Time)> AgeStatus;
    function<Alive(void)> AliveStatus;
    function<CD4(Time)> CD4Count;
    function<HIVStatus(void)> GetHIVStatus;
    function<bool(void)> ARTStatus;
    function<double(Time)> GlobalTBPrevalence;
    function<double(void)> HouseholdTBPrevalence;
    function<double(TBStatus)> ContactHouseholdTBPrevalence;

    function<shared_p<TB>(void)> GetLifetimePtr;

    //////////////////////////////////////////////////////////////////////////
    // Event handler functions
    //////////////////////////////////////////////////////////////////////////

    function<void(Time)> DeathHandler;   
    function<void(Time)> ProgressionHandler;
    function<ContactTraceResult(const Time&, Param&, Param&, RNG&)>  ContactTraceHandler;
    function<void(Time)> RecoveryHandler;
};
