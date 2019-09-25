#include "../../include/TBABM/TBABM.h"
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace StatisticalDistributions;
using std::vector;
using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

EventFunc TBABM::VCTDiagnosis(weak_p<Individual> idv_w)
{
  EventFunc ef = 
    [this, idv_w](double t, SchedulerT scheduler) {
      // printf("[%d] VCTDiagnosis: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));

      int startYear     = constants["startYear"];
      int ageGroupWidth = constants["ageGroupWidth"];

      // Make sure the individual is not dead, and has not
      // already been diagnosed
      auto idv = idv_w.lock();
      if (!idv)
        return true;
      if (idv->dead || idv->hivDiagnosed) {
        // printf("\tDead or already diagnosed\n");
        return true;
      }

      // Get constant parameter m_30 and CD4
      double m_30 = params["HIV_m_30"].Sample(rng);
      double CD4  = idv->CD4count(t, m_30);

      // Retrieve a_t_i (varies by year and age group) and sig_i
      //   (varies by age group)
      double a_t_i = fileData["HIV_a_t_i"].getValue(startYear+(int)t/365, 0, idv->age(t), rng);
      double sig_i = fileData["HIV_sig_i"].getValue(0, 0, idv->age(t), rng);

      // Calculate rate of diagnosis and time to diagnosis
      double D_c = a_t_i*exp(-1 * sig_i * CD4);
      // printf("CD4: %f\t\ta_t_i: %f\t\tD_c: %f\n", CD4, a_t_i, D_c);
      double timeToDiagnosis = Exponential(D_c)(rng.mt_);

      bool initiateART;
      if (idv->tb.GetTBStatus(t) == TBStatus::Infectious)
        initiateART = fileData["HIV_p_art_tb"].getValue(0, 0, CD4, rng);
      else
        initiateART = fileData["HIV_p_art"].getValue(0, 0, CD4, rng);

      // printf("\tInitiateART: %d\n", (int)initiateART);
      // printf("\tCD4: %d\n", (int)idv->CD4count(t, m_30));

      if (timeToDiagnosis < ageGroupWidth && idv->hivStatus == HIVStatus::Positive) {

        idv->hivDiagnosed = true;
        data.hivDiagnosed.Record(t, +1);
        data.hivDiagnosedVCT.Record(t, +1);
        data.hivDiagnosesVCT.Record(t, +1);

        if (ARTEligible(t, idv) && initiateART) {
          // printf("\tART eligible\n");
          Schedule(t + 365*timeToDiagnosis, ARTInitiate(idv_w));
        }
        else if (!ARTEligible(t, idv)) {
          // printf("\tART ineligible\n");
          seekingART.push_back(idv_w);
        }
        else {
          // printf("\tART eligible, but not initiating\n");
          seekingART.push_back(idv_w);
        }
      }
      else {
        // printf("\tNot diagnosed during this period\n");
        Schedule(t + 365*ageGroupWidth, VCTDiagnosis(idv_w));
      }

      return true;
    };

  return ef;
}
