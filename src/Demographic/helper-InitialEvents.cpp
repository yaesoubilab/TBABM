#include "../../include/TBABM/TBABM.h"
#include <Exponential.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using namespace StatisticalDistributions;
using Sex = Individual::Sex;

// Unit of dt is years
void TBABM::InitialEvents(Pointer<Individual> idv, double t, double dt)
{
	int gender = idv->sex == Sex::Male ? 0 : 1;
	double age = idv->age(t);
	auto startYear = constants["startYear"];

	// Unit of both of these is years
	double timeToDeath   = fileData["naturalDeath"].getValue(startYear+(int)t/365, gender, age, rng);
	double timeToLooking = fileData["timeToLooking"].getValue(0, gender, age, rng);

	if (timeToDeath < dt)
		Schedule(t + 365*timeToDeath, Death(idv));

	if ((idv->marriageStatus == MarriageStatus::Single ||
		 idv->marriageStatus == MarriageStatus::Divorced) &&
		 timeToLooking < dt)
		Schedule(t + 365*timeToLooking, SingleToLooking(idv));


	return;
}