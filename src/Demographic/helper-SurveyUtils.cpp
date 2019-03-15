#include "../../include/TBABM/SurveyUtils.h"


using MS = MarriageStatus;

using std::to_string;

string Ihash(IPt idv) {
	return to_string(std::hash<IPt>()(idv));
}

string Hhash(HPt hh) {
	return to_string(std::hash<HPt>()(hh));
}


string age(IPt idv, int t) {
	return to_string(idv->age(t));
}

string sex(IPt idv) {
	return idv->sex == Sex::Male ? "male" : "female";
}

string marital(IPt idv) {
	switch (idv->marriageStatus) {
		case MS::Single:   return "single";   break;			
		case MS::Married:  return "married";  break;
		case MS::Divorced: return "divorced"; break;
		case MS::Looking:  return "looking";  break;
		default:		   return "UNSUPPORTED MARITAL";
	}
}

string numChildren(IPt idv) {
	return to_string(idv->numOffspring());
}

string mom(IPt idv) {
	assert(idv);

	if (!idv->mother.use_count())
		return "dead";
	else
		return idv->mother->dead ? "dead" : "alive";
}

string dad(IPt idv) {
	assert(idv);

	if (!idv->father.use_count())
		return "dead";
	else
		return idv->father->dead ? "dead" : "alive";
}


string causeDeath(DeathCause cause_death) {
	switch (cause_death) {
		case DeathCause::HIV:      return "HIV";      break;			
		case DeathCause::Natural:  return "natural";  break;
		case DeathCause::TB:       return "TB";       break;
		default:		           return "UNSUPPORTED cause of death";
	}
}

string HIV(IPt idv) {
	return idv->hivStatus == HIVStatus::Positive ? "true" : "false";
}

string HIV_date(IPt idv) {
	if (idv->hivStatus == HIVStatus::Positive)
		return to_string(idv->t_HIV_infection);
	else
		return to_string(0);
}

string ART(IPt idv) {
	return idv->onART ? "true" : "false";
}

string ART_date(IPt idv) {
	return to_string(idv->onART ? idv->ARTInitTime : 0);
}

string CD4(IPt idv, double t, double m_30) {
	return to_string(idv->CD4count(t, m_30));
}

string ART_baseline_CD4(IPt idv, double m_30) {
	return to_string(idv->onART ? idv->ART_init_CD4 : 0);
}

string TBStatus(IPt idv, int t) {
	switch (idv->tb.GetTBStatus(t)) {
		case TBStatus::Susceptible: return "Susceptible"; break;
		case TBStatus::Latent:		return "Latent"; break;
		case TBStatus::Infectious:  return "Infectious"; break;
		default:				    return "UNSUPPORTED TBStatus";
	}
}