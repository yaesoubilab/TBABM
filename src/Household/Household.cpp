#include "../../include/TBABM/Household.h"

void Household::AddIndividual(shared_p<Individual> idv, int t, HouseholdPosition hp) {

  if (!idv)
    return;

  // Avoid adding a dead individual to the household
  if (idv->dead) {
    printf("DEAD INDIVIDUAL WAS ALMOST ADDED!!!!!!!\n");;
    return;
  }

  // Update role and HID for new household member
  idv->householdPosition = hp;
  idv->householdID 	   = hid;

  // Update 'livedWithBefore' for new member, and all current residents
  idv->LivedWith(head);
  idv->LivedWith(spouse);

  if (head)   head->LivedWith(idv);
  if (spouse) spouse->LivedWith(idv);

  for (auto it = offspring.begin(); it != offspring.end(); it++) {
    if (!*it || (*it)->dead) continue;
    (*it)->LivedWith(idv);
    idv->LivedWith(*it);
  }

  for (auto it = other.begin(); it != other.end(); it++) {
    if (!*it || (*it)->dead) continue;
    (*it)->LivedWith(idv);
    idv->LivedWith(*it);
  }

  // Insert new member into the population
  if (hp == HouseholdPosition::Head) {
    if (head) {
      head->householdPosition = HouseholdPosition::Other;
      other.push_back(head);
    }
    head = idv;
  }
  else if (hp == HouseholdPosition::Spouse) {
    if (spouse) {
      spouse->householdPosition = HouseholdPosition::Other;
      other.push_back(spouse);
    }
    spouse = idv;
  }
  else if (hp == HouseholdPosition::Other) {
    other.push_back(idv);
  }
  else if (hp == HouseholdPosition::Offspring) {
    offspring.push_back(idv);
  }

  nIndividuals += 1;

  if (idv->tb.GetTBStatus(t) == TBStatus::Infectious)
    nInfectiousTBIndivduals += 1;

  idv->tb.SetHouseholdCallbacks(
    [this, idv] (int t) -> int    { return ContactTrace(t, idv); },

    [this, idv] (int t) -> void   { nInfectiousTBIndivduals += 1;
                                    TriggerReeval(t, idv); },

    [this]      (int)   -> void   { nInfectiousTBIndivduals -= 1; },

    [this]      (void)  -> double { return ActiveTBPrevalence(); },

    [this]      (TBStatus s)  -> double { return ContactActiveTBPrevalence(s); }
  );

  return;
}

void Household::RemoveIndividual(weak_p<Individual> idv_w, int t) {
  auto idv = idv_w.lock();
  if (!idv)
    assert(false);

  if (!hasMember(idv))
    return;
  if (head == idv) {
    head.reset();
    // Find a new head
    if (spouse) {
      head = spouse;
      head->householdPosition = HouseholdPosition::Head;
      spouse.reset();
    } else if (offspring.size() > 0) {
      for (auto it = offspring.begin(); it != offspring.end(); it++) {
        if (*it && !(*it)->dead) {
          head = *it;
          head->householdPosition = HouseholdPosition::Head;
          offspring.erase(it);
          break;
        }
      }
    } else if (other.size() > 0) {
      for (auto it = other.begin(); it != other.end(); it++) {
        if (*it && !(*it)->dead) {
          head = *it;
          head->householdPosition = HouseholdPosition::Head;
          other.erase(it);
          break;
        }
      }
    } else {
      // There is nobody to replace the head.
    }
  }

  if (spouse == idv)
    spouse.reset();

  for (auto it = offspring.begin(); it != offspring.end(); it++) {
    if (*it == idv) {
      offspring.erase(it);
      break;
    }
  }

  for (auto it = other.begin(); it != other.end(); it++)
    if (*it == idv) {
      other.erase(it);
      break;
    }

  nIndividuals -= 1;
  if (idv->tb.GetTBStatus(t) == TBStatus::Infectious)
    nInfectiousTBIndivduals -= 1;

  idv->tb.ResetHouseholdCallbacks();

  return;
}

void Household::PrintHousehold(int t) {
  printf("[%d] Printing household\n", t);
  if (head)
    printf("\t[H]  %c %d ", head->sex == Sex::Male ? 'M' : 'F', (t-head->birthDate)/365);

  if (spouse)
    printf("\t[S]  %c %d ", spouse->sex == Sex::Male ? 'M' : 'F', (t-spouse->birthDate)/365);

  for (auto it = offspring.begin(); it != offspring.end(); it++) {
    assert(*it);
    printf("\t[Of] %c %d ", (*it)->sex == Sex::Male ? 'M' : 'F', (t-(*it)->birthDate)/365);
  }

  for (auto it = other.begin(); it != other.end(); it++) {
    assert(*it);
    printf("\t[Ot] %c %d ", (*it)->sex == Sex::Male ? 'M' : 'F', (t-(*it)->birthDate)/365);
  }
  printf("\n");
}

int Household::size(int t) {
  return size();
}

int Household::size(void) {
  return nIndividuals;
}

bool Household::hasMember(weak_p<Individual> idv_w) {
  auto idv = idv_w.lock();
  if (!idv)
    return false;

  if ((head == idv) || (spouse == idv))
    return true;

  for (auto candidate : offspring)
    if (candidate == idv)
      return true;

  for (auto candidate : other)
    if (candidate == idv)
      return true;

  return false;
}

double Household::ActiveTBPrevalence() {
  assert(nIndividuals >= nInfectiousTBIndivduals);
  assert(nIndividuals > 0);

  return nInfectiousTBIndivduals / nIndividuals;
}

double Household::ActiveTBPrevalence(int t) {
  return ActiveTBPrevalence();
}

double Household::ContactActiveTBPrevalence(TBStatus s) {
  if(nIndividuals < nInfectiousTBIndivduals) {
    printf("warn\n");
    return 1;
  }
  assert(nIndividuals > 0);

  if (nIndividuals == 1)
    return 0;

  if (s == TBStatus::Infectious)
    return (nInfectiousTBIndivduals-1) / ((double)size() - 1);
  else
    return nInfectiousTBIndivduals / ((double)size() - 1);
}

double Household::ContactActiveTBPrevalence(TBStatus s, int t) {
  return ContactActiveTBPrevalence(s);
}

int Household::ContactTrace(int t, shared_p<Individual> idv) {

  int cases_found {0};

  if (head && head != idv)
    cases_found += head->tb.ContactTrace(t);
  if (spouse && spouse != idv)
    cases_found += spouse->tb.ContactTrace(t);

  for (auto it = offspring.begin(); it != offspring.end(); it++) {
    assert(*it);
    if (*it != idv)
      cases_found += (*it)->tb.ContactTrace(t);
  }

  for (auto it = other.begin(); it != other.end(); it++) {
    assert(*it);
    if (*it != idv)
      cases_found += (*it)->tb.ContactTrace(t);
  }

  return cases_found;
}

void Household::TriggerReeval(int t, weak_p<Individual> idv_w) {
  auto idv = idv_w.lock();
  if (!idv)
    return;

  if (head && head != idv)
    head->tb.RiskReeval(t);

  if (spouse && spouse != idv)
    spouse->tb.RiskReeval(t);

  for (auto person : offspring)
    if (person && person != idv)
      person->tb.RiskReeval(t);

  for (auto person : other)
    if (person && person != idv)
      person->tb.RiskReeval(t);

  return;
}
