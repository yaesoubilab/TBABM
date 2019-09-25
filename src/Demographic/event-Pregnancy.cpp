#include "../../include/TBABM/TBABM.h"

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

EventFunc TBABM::Pregnancy(weak_p<Individual> mother_w, 
    weak_p<Individual> father_w)
{
  EventFunc ef = 
    [this, mother_w, father_w] (double t, SchedulerT scheduler) {

      auto mother = mother_w.lock();
      auto father = father_w.lock();

      if (!mother)
        return true;
      if (mother->dead || mother->pregnant)
        return true;

      mother->pregnant = true;

      Schedule(t + 9*30, Birth(mother_w, father_w));

      return true;
    };

  return ef;
}
