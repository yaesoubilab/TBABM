#include "../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using namespace StatisticalDistributions;

  MasterData
TBABM::GetData(void)
{
  return data;
}

  bool
TBABM::WriteSurveys(shared_p<ofstream> ps, 
    shared_p<ofstream> hs, 
    shared_p<ofstream> ds)
{
  bool fail = false;

  *ps << populationSurvey;
  *hs << householdSurvey;
  *ds << deathSurvey;

  if (ps->fail()) {
    fail = true;
    printf("Write to populationSurvey failed\n");
  }

  if (hs->fail()) {
    fail = true;
    printf("Write to householdSurvey failed\n");
  }

  if (ds->fail()) {
    fail = true;
    printf("Write to deathSurvey failed\n");
  }

  return !fail;
}

bool TBABM::Run(void)
{
  CreatePopulation(0, constants["populationSize"]);
  Schedule(1, Matchmaking());
  Schedule(1, UpdatePyramid());
  Schedule(1, UpdateHouseholds());
  Schedule(1, ARTGuidelineChange());
  Schedule(1, Survey());

  // Schedule(1, ExogenousBirth());

  int events_processed {0};

  while (!eq.Empty()) {
    auto e = eq.Top();
//    assert(e);

    if (e.t > constants["tMax"])
      break;

    eq.Pop();
    e.run();
    events_processed += 1;
  }

  // Pop off all the events that were greater than tMax
  while (!eq.Empty())
    eq.Pop();

  // printf("Events processed: %d\n", events_processed);

  for (size_t i = 0; i < households.size(); i++)
    households[i].reset();

  for (size_t i = 0; i < population.size(); i++)
    population[i].reset();

  data.Close();

  return true;
}

void TBABM::Schedule(int t, EventQueue<>::EventFunc ef)
{
  auto f = eq.MakeScheduledEvent(t, ef);
  eq.Schedule(f);
  return;
}

void TBABM::PurgeReferencesToIndividual(weak_p<Individual> host_w,
    weak_p<Individual> idv_w)
{
  // Return if host or individual do not exist
  auto host = host_w.lock();
  auto idv = idv_w.lock();
  if (!host || !idv)
    return;

  // Reset pointers for spouse, mother, father (rough equivalent of setting
  // pointers to NULL)
  if (host->spouse.lock() && host->spouse.lock() == idv)
    host->spouse.reset();
  if (host->mother.lock() && host->mother.lock() == idv)
    host->mother.reset();
  if (host->father.lock() && host->father.lock() == idv)
    host->father.reset();

  // Erase 'idv' from offspring
  for (auto it = host->offspring.begin(); it != host->offspring.end(); it++)
    if ((*it).lock() == idv) {
      host->offspring.erase(it);
      break;
    }

  // Erase 'idv' from list of people 'idv' has lived with before
  for (auto it = host->livedWithBefore.begin(); it != host->livedWithBefore.end(); it++)
    if ((*it).lock() == idv) {
      it = host->livedWithBefore.erase(it);
      break;
    }
}

void TBABM::DeleteIndividual(weak_p<Individual> idv_w)
{
  auto idv = idv_w.lock();
  assert(idv);

  for (size_t i = 0; i < idv->livedWithBefore.size(); i++)
    PurgeReferencesToIndividual(idv->livedWithBefore[i], idv);
}

void TBABM::ChangeHousehold(weak_p<Individual> idv_w, int t, int newHID, HouseholdPosition newRole)
{
  auto idv = idv_w.lock();
  if (!idv)
    return;
  if (idv->dead)
    return;

  // printf("\tChanging household of %ld::%lu\n", idv->householdID, std::hash<Pointer<Individual>>()(idv));
  int oldHID = idv->householdID;

  auto oldHousehold = households[oldHID];
  auto newHousehold = households[newHID];

  assert(idv);
  assert(oldHousehold);
  assert(newHousehold);

  if (oldHousehold != newHousehold)
    oldHousehold->RemoveIndividual(idv, t);

  newHousehold->AddIndividual(idv, t, newRole);

  // Clear household if there's nobody left in it
  if (oldHousehold->size() == 0)
    households[oldHID].reset();

  return;
}
