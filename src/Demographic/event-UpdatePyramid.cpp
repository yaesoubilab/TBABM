#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using Sex = Individual::Sex;

using namespace StatisticalDistributions;

EventFunc TBABM::UpdatePyramid(void)
{
	EventFunc ef = 
		[this] (double t, SchedulerT scheduler) {
			for (auto it = population.begin(); it != population.end(); it++) {
				if (!*it || (*it)->dead)
					continue;

				auto idv = *it;

				int age = idv->age(t);
				int sex = idv->sex == Sex::Male ? 0 : 1;

				pyramid.UpdateByAge(t-1, sex, age, +1);
			}

			if (t == 1) {
				ofstream f;
				f.open("../output/histogramT1.csv");
				f << string("household\n");
				for (auto it = households.begin(); it != households.end(); it++) {
					if (it->second)
						f << to_string(it->second->size()).c_str() << "\n";
				}
				f.close();
			}

			if (t == 1+365*9) {
				ofstream f;
				f.open("../output/histogramT10.csv");
				f << string("household\n");
				for (auto it = households.begin(); it != households.end(); it++)
					if (it->second && it->second->size() > 0)
						f << to_string(it->second->size()).c_str() << "\n";
				f.close();
			}

			if (t == 1+365*19) {
				ofstream f;
				f.open("../output/histogramT19.csv");
				f << string("household\n");
				for (auto it = households.begin(); it != households.end(); it++)
					if (it->second && it->second->size() > 0)
						f << to_string(it->second->size()).c_str() << "\n";
				f.close();
			}

			Schedule(t + 365, UpdatePyramid());
			return true;
		};

	return ef;
}