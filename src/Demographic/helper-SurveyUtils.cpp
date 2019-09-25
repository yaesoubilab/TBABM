#include "../../include/TBABM/SurveyUtils.h"

using MS = MarriageStatus;

using std::to_string;

string Ihash(shared_p<Individual> idv) {
  return to_string(std::hash<shared_p<Individual>>()(idv));
}

string Hhash(shared_p<Household> hh) {
  return to_string(std::hash<shared_p<Household>>()(hh));
}


string age(shared_p<Individual> idv, int t) {
  return to_string(idv->age(t));
}

string sex(shared_p<Individual> idv) {
  return idv->sex == Sex::Male ? "male" : "female";
}

string marital(shared_p<Individual> idv) {
  switch (idv->marriageStatus) {
    case MS::Single:   return "single";   break;			
    case MS::Married:  return "married";  break;
    case MS::Divorced: return "divorced"; break;
    case MS::Looking:  return "looking";  break;
    default:		   return "UNSUPPORTED MARITAL";
  }
}

string numChildren(shared_p<Individual> idv) {
  return to_string(idv->numOffspring());
}

string mom(shared_p<Individual> idv) {
  assert(idv);

  auto mom = idv->mother.lock();
  if (!mom)
    return "dead";
  else
    return mom->dead ? "dead" : "alive";
}

string dad(shared_p<Individual> idv) {
  assert(idv);

  auto father = idv->father.lock();
  if (!father)
    return "dead";
  else
    return father->dead ? "dead" : "alive";
}


string causeDeath(DeathCause cause_death) {
  switch (cause_death) {
    case DeathCause::HIV:      return "HIV";      break;			
    case DeathCause::Natural:  return "natural";  break;
    case DeathCause::TB:       return "TB";       break;
    default:		           return "UNSUPPORTED cause of death";
  }
}

string HIV(shared_p<Individual> idv) {
  return idv->hivStatus == HIVStatus::Positive ? "true" : "false";
}

string HIV_date(shared_p<Individual> idv) {
  if (idv->hivStatus == HIVStatus::Positive)
    return to_string(idv->t_HIV_infection);
  else
    return to_string(0);
}

string ART(shared_p<Individual> idv) {
  return idv->onART ? "true" : "false";
}

string ART_date(shared_p<Individual> idv) {
  return to_string(idv->onART ? idv->ARTInitTime : 0);
}

string CD4(shared_p<Individual> idv, double t, double m_30) {
  return to_string(idv->CD4count(t, m_30));
}

string ART_baseline_CD4(shared_p<Individual> idv, double m_30) {
  return to_string(idv->onART ? idv->ART_init_CD4 : 0);
}

string TBStatus(shared_p<Individual> idv, int t) {
  switch (idv->tb.GetTBStatus(t)) {
    case TBStatus::Susceptible: return "Susceptible"; break;
    case TBStatus::Latent:		return "Latent"; break;
    case TBStatus::Infectious:  return "Infectious"; break;
    default:				    return "UNSUPPORTED TBStatus";
  }
}
