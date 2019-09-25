#pragma once

#include <cstdio>
#include <iostream>
#include <map>
#include <string>

#include "Household.h"
#include "Individual.h"
#include "IndividualTypes.h"
#include "Names.h"
#include "Pointers.h"

#include <StatisticalDistribution.h>
#include <EventQueue.h>
#include <RNG.h>

using std::map;
using std::string;

using namespace StatisticalDistributions;
using EQ = EventQueue<double, bool>;

class HouseholdGen {
  public:
    using Distributions = map<string, shared_p<StatisticalDistribution<long double>>>;
    using Constants = map<string, double>;

    struct MicroIndividual {
      HouseholdPosition role;
      Sex sex;
      int age;
    };

    using MicroFamily = std::vector<MicroIndividual>;

    shared_p<Household> GetHousehold(int current_time, int hid, RNG &rng);

    HouseholdGen(const char *file,
        Params& (params),
        map<string, DataFrameFile>& (fileData),
        EQ& event_queue,
        std::function<IndividualInitData()> GetInitData,
        IndividualHandlers handles) : 
      file(file), params(params), fileData(fileData), 
      event_queue(event_queue), GetInitData(GetInitData),
      initHandles(handles) {
        FILE *ifile = fopen(file, "r");
        int c;
        int lines = 2;

        // Skip first two lines
        while ((c = getc(ifile)) != EOF && c != '\n');
        while ((c = getc(ifile)) != EOF && c != '\n');

        while (!feof(ifile)) {
          lines++;

          fscanf(ifile, "%i", &c);
          if (getc(ifile) != ',') {
            std::cerr << "Line #" << lines << " has a noninteger number of people. Skipping over" << std::endl;
            while ((c = getc(ifile)) != EOF && c != '\n');
            continue;
          }

          MicroFamily f;
          while (c--) {
            MicroIndividual idv;
            int r, s; // Hold temporary values

            HouseholdPosition role;
            Sex sex;
            int age;

            fscanf(ifile, "%i,%i,%i,", &r, &s, &age);

            switch (r) {
              case (1):  role = HouseholdPosition::Head; break;
              case (2):  role = HouseholdPosition::Spouse; break;
              case (3):  role = HouseholdPosition::Offspring; break;
              case (4):  role = HouseholdPosition::Offspring; break;
              case (5):  role = HouseholdPosition::Other; break;
              case (6):  role = HouseholdPosition::Other; break;
              case (7):  role = HouseholdPosition::Other; break;
              case (8):  role = HouseholdPosition::Other; break;
              case (9):  role = HouseholdPosition::Other; break;
              case (10): role = HouseholdPosition::Other; break;
              case (11): role = HouseholdPosition::Other; break;
              case (12): role = HouseholdPosition::Other; break;
              case (13): role = HouseholdPosition::Other; break;
              case (98): role = HouseholdPosition::Other; break;
              case (99): role = HouseholdPosition::Other; break;
              default:   role = HouseholdPosition::Other;
            }

            switch (s) {
              case(1): sex = Sex::Male;   break;
              case(2): sex = Sex::Female; break;
              default: sex = Sex::Male;
            }

            idv.role = role;
            idv.sex = sex;
            idv.age = age;

            f.push_back(idv);
          }

          int b;
          while((b = getc(ifile)) != EOF && b != '\n');
          families.push_back(f);
        }
      };
  private:
    std::vector<MicroFamily> families;

    Params& params;
    map<string, DataFrameFile>& fileData;

    EQ& event_queue;
    std::function<IndividualInitData()> GetInitData;
    IndividualHandlers initHandles;
    Names name_gen;

    const char *file;
};
